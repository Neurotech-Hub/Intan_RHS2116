/***************************************************************************/ /**
																			   * @file rhs2116.c
																			   * @brief Intan RHS2116 library
																			   ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "rhs2116.h"
#include "spidrv.h"

static Rhs2116_Context_t rhs2116_context;
uint32_t tx_buffer;
uint32_t rx_buffer;

// Flag to signal that transfer is complete
static volatile bool transfer_complete = false;

// Callback fired when data is transmitted
void transfer_callback(SPIDRV_HandleData_t *handle, Ecode_t transfer_status,
					   int items_transferred)
{
	(void)&handle;
	(void)items_transferred;

	// Post semaphore to signal to application
	// task that transfer is successful
	if (transfer_status == ECODE_EMDRV_SPIDRV_OK)
	{
		transfer_complete = true;
	}
}

void rhs2116_init(SPIDRV_Handle_t spiHandle)
{
	rhs2116_context.spiHandle = spiHandle;
}

uint16_t do_transfer(void)
{
	Ecode_t ecode;
	transfer_complete = false;
	ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &tx_buffer, &rx_buffer,
							 sizeof(tx_buffer), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;

	// dummy cycles
	tx_buffer = 0;
	rx_buffer = 0;
	transfer_complete = false;
	ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &tx_buffer, &rx_buffer,
							 sizeof(tx_buffer), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;

	rx_buffer = 0; // tx is still clear
	transfer_complete = false;
	ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &tx_buffer, &rx_buffer,
							 sizeof(tx_buffer), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;

	// get bytes back in order
	uint16_t receivedData = ((rx_buffer >> 8) & 0xFF00) | ((rx_buffer >> 24) & 0xFF);

	return receivedData;
}

bool rhs2116_writeRegister(uint8_t regAddress, uint16_t regValue, bool uFlag,
						   bool mFlag)
{
	tx_buffer = 0;

	// Split regValue into MSB and LSB
	uint8_t dataMSB = (regValue >> 8) & 0xFF; // Extract MSB of the data
	uint8_t dataLSB = regValue & 0xFF;		  // Extract LSB of the data

	// Construct the LSB byte of the command with the specified structure
	uint8_t lsbCommand = 0x80; // Bit 7 set to 1, bits 6-0 set to 0 as base
	if (uFlag)
	{
		lsbCommand |= 0x20; // Set U flag (bit 5)
	}
	if (mFlag)
	{
		lsbCommand |= 0x10; // Set M flag (bit 4)
	}

	// Assemble the command with dataLSB in the most significant position
	tx_buffer = ((uint32_t)dataLSB << 24) | ((uint32_t)dataMSB << 16) | ((uint32_t)regAddress << 8) | lsbCommand;

	uint16_t receivedData = do_transfer();

	// Check if the received value matches the sent value
	if (receivedData == regValue)
	{
		return true; // Data integrity check passed
	}
	return false; // Data integrity check failed
}

uint16_t rhs2116_readRegister(uint8_t regAddress, bool uFlag, bool mFlag)
{
	tx_buffer = 0;

	uint8_t lsbCommand = 0xC0; // Bits 7 and 6 set to 1, bits 5-0 set to 0 as base
	if (uFlag)
	{
		lsbCommand |= 0x20; // Set U flag (bit 5)
	}
	if (mFlag)
	{
		lsbCommand |= 0x10; // Set M flag (bit 4)
	}

	tx_buffer = ((uint32_t)regAddress << 8) | (uint32_t)lsbCommand;
	uint16_t receivedData = do_transfer();
	return receivedData;
}

void rhs2116_clear(void)
{
	// Clear the buffers
	tx_buffer = RHS_CLEAR;
	rx_buffer = 0;
	do_transfer();
}

bool rhs2116_checkId(void)
{
	uint16_t chipId = rhs2116_readRegister(RHS_CHIP_ID, false, false);
	if (chipId == CHIP_ID)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Configures Register 0: Supply Sensor and ADC Buffer Bias Current
 * MUX bias [5:0]: Configures the bias current of the MUX (function of ADC sampling rate).
 * ADC buffer bias [5:0]: Configures the bias current of the internal reference buffer in the ADC (function of ADC sampling rate).
 */
uint16_t rhs2116_regValue_RHS_SUPPS_BIASCURR(uint8_t adcBufferBias, uint8_t muxBias)
{
	// Ensure the values fit in their respective fields
	adcBufferBias &= 0x3F; // Mask to 6 bits
	muxBias &= 0x3F;	   // Mask to 6 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (adcBufferBias << 6) | (muxBias << 0);

	return command;
}

/*
 * Configures Register 1: ADC Output Format, DSP Offset Removal, and Auxiliary Digital Outputs
 * DSP cutoff freq [3:0]: Sets the cutoff frequency of the DSP filter for offset removal.
 * DSPen: Enables DSP offset removal when set to 1.
 * absmode: Passes all amplifier ADC conversions through an absolute value function when set to 1.
 * twoscomp: Reports AC high-gain amplifier conversions using two's complement representation when set to 1.
 * weak MISO: Puts MISO line into high impedance mode when CS is high and LVDS_en is low if set to 0.
 * digout1 HiZ: Puts auxout1 into high impedance mode when set to 1.
 * digout1: Drives auxout1 with this bit value when digout1 HiZ is 0.
 * digout2 HiZ: Puts auxout2 into high impedance mode when set to 1.
 * digout2: Drives auxout2 with this bit value when digout2 HiZ is 0.
 * digoutOD: Controls open-drain auxiliary high-voltage digital output pin auxoutOD.
 */
uint16_t rhs2116_regValue_RHS_OUTFMT_DSP_AUXDIO(uint8_t dspCutoffFreq, bool dspEn, bool absMode, bool twosComp, bool weakMiso, bool digout1HiZ, bool digout1, bool digout2HiZ, bool digout2, bool digoutOD)
{
	// Ensure the values fit in their respective fields
	dspCutoffFreq &= 0xF; // Mask to 4 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (digoutOD << 12) | (digout2 << 11) | (digout2HiZ << 10) | (digout1 << 9) | (digout1HiZ << 8) | (weakMiso << 7) | (twosComp << 6) | (absMode << 5) | (dspEn << 4) | (dspCutoffFreq << 0);

	return command;
}

// Define the positions and masks for the fields
#define ZCHECK_SELECT_POS 8
#define ZCHECK_SELECT_MASK 0x3F // Binary: 00111111
#define ZCHECK_DAC_POWER_POS 7
#define ZCHECK_LOAD_POS 6
#define ZCHECK_SCALE_POS 4
#define ZCHECK_SCALE_MASK 0x3 // Binary: 00000011
#define ZCHECK_EN_POS 0

/*
 * Configures Register 2: Impedance Check Control
 * Zcheck en: Activates impedance testing mode when set to 1.
 * Zcheck scale [1:0]: Selects the series capacitor for AC current waveform generation (00 = 0.1 pF, 01 = 1.0 pF, 11 = 10 pF).
 * Zcheck load: Adds a capacitor load to the impedance checking network when set to 1 (should be 0 for normal operation).
 * Zcheck DAC power: Activates the on-chip DAC for impedance measurement when set to 1.
 * Zcheck select [5:0]: Selects the electrode for impedance testing.
 */
uint16_t rhs2116_regValue_RHS_IMPCHK_CTRL(uint8_t zcheckSelect, bool zcheckDacPower, bool zcheckLoad, uint8_t zcheckScale, bool zcheckEn)
{
	// Ensure the values fit in their respective fields
	zcheckSelect &= 0x3F; // Mask to 6 bits
	zcheckScale &= 0x3;	  // Mask to 2 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (zcheckSelect << 8) | (zcheckDacPower << 7) | (zcheckLoad << 6) | (zcheckScale << 4) | (zcheckEn << 0);

	return command;
}

/*
 * Configures Register 3: Impedance Check DAC
 * Zcheck DAC [7:0]: Sets the output voltage of the DAC for impedance checking.
 */
uint16_t rhs2116_regValue_IMPCHK_DAC(uint8_t zcheckDac)
{
	// Ensure the value fits in its respective field
	zcheckDac &= 0xFF; // Mask to 8 bits

	// Construct the command by placing the value in the correct position
	uint16_t command = zcheckDac;

	return command;
}

/*
 * Configures Register 4: RH1 Cutoff Frequency
 * RH1 sel1 [5:0], RH1 sel2 [4:0]: Sets the upper cutoff frequency of the biopotential amplifiers.
 */
uint16_t rhs2116_regValue_RH1_CUTOFF(uint8_t rh1Sel1, uint8_t rh1Sel2)
{
	// Ensure the values fit in their respective fields
	rh1Sel1 &= 0x3F; // Mask to 6 bits
	rh1Sel2 &= 0x1F; // Mask to 5 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (rh1Sel2 << 6) | rh1Sel1;

	return command;
}

/*
 * Configures Register 5: RH2 Cutoff Frequency
 * RH2 sel1 [5:0], RH2 sel2 [4:0]: Sets the upper cutoff frequency of the biopotential amplifiers.
 */
uint16_t rhs2116_regValue_RH2_CUTOFF(uint8_t rh2Sel1, uint8_t rh2Sel2)
{
	// Ensure the values fit in their respective fields
	rh2Sel1 &= 0x3F; // Mask to 6 bits
	rh2Sel2 &= 0x1F; // Mask to 5 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (rh2Sel2 << 6) | rh2Sel1;

	return command;
}

/*
 * Configures Register 6: RL_A Cutoff Frequency
 * RL_A sel1 [6:0], RL_A sel2 [5:0], RL_A sel3: Sets the "A version" of the lower cutoff frequency of the biopotential amplifiers.
 */
uint16_t rhs2116_regValue_RL_A_CUTOFF(uint8_t rlASel1, uint8_t rlASel2, bool rlASel3)
{
	// Ensure the values fit in their respective fields
	rlASel1 &= 0x7F; // Mask to 7 bits
	rlASel2 &= 0x3F; // Mask to 6 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (rlASel3 << 13) | (rlASel2 << 7) | rlASel1;

	return command;
}

/*
 * Configures Register 7: RL_B Cutoff Frequency
 * RL_B sel1 [6:0], RL_B sel2 [5:0], RL_B sel3: Sets the "B version" of the lower cutoff frequency of the biopotential amplifiers.
 */
uint16_t rhs2116_regValue_RL_B_CUTOFF(uint8_t rlBSel1, uint8_t rlBSel2, bool rlBSel3)
{
	// Ensure the values fit in their respective fields
	rlBSel1 &= 0x7F; // Mask to 7 bits
	rlBSel2 &= 0x3F; // Mask to 6 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (rlBSel3 << 13) | (rlBSel2 << 7) | rlBSel1;

	return command;
}

/*
 * Configures Register 8: Individual AC Amplifier Power
 * AC amp power [15:0]: Powers down individual AC-coupled high-gain amplifiers when set to 0.
 */
uint16_t rhs2116_regValue_ACAMP_PWR(uint16_t acAmpPower)
{
	// Ensure the value fits in its respective field
	acAmpPower &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the acAmpPower value
	uint16_t command = acAmpPower;

	return command;
}

/*
 * Configures Register 10: Amplifier Fast Settle (TRIGGERED REGISTER)
 * amp fast settle [15:0]: Drives AC high-gain amplifier outputs to baseline level when set to 1.
 * Note: Register 10 is a triggered register.
 */
uint16_t rhs2116_regValue_AMP_FSTSETL(uint16_t ampFastSettle)
{
	// Ensure the value fits in its respective field
	ampFastSettle &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the ampFastSettle value
	uint16_t command = ampFastSettle;

	return command;
}

/*
 * Configures Register 12: Amplifier Lower Cutoff Frequency Select (TRIGGERED REGISTER)
 * amp fL select [15:0]: Selects between two different lower cutoff frequencies for each AC high-gain amplifier.
 * Note: Register 12 is a triggered register.
 */
uint16_t rhs2116_regValue_AMP_LCUTOFF(uint16_t ampFLSelect)
{
	// Ensure the value fits in its respective field
	ampFLSelect &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the ampFLSelect value
	uint16_t command = ampFLSelect;

	return command;
}

/*
 * Configures Register 32: Stimulation Enable A
 * stim enable A [15:0]: Must be set to 0xAAAA to enable on-chip stimulators.
 */
uint16_t rhs2116_regValue_STIM_EN_A(uint16_t stimEnableA)
{
	// Ensure the value fits in its respective field
	stimEnableA &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the stimEnableA value
	uint16_t command = stimEnableA;

	return command;
}

/*
 * Configures Register 33: Stimulation Enable B
 * stim enable B [15:0]: Must be set to 0x00FF to enable on-chip stimulators.
 */
uint16_t rhs2116_regValue_STIM_EN_B(uint16_t stimEnableB)
{
	// Ensure the value fits in its respective field
	stimEnableB &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the stimEnableB value
	uint16_t command = stimEnableB;

	return command;
}

/*
 * Configures Register 34: Stimulation Current Step Size
 * step sel1 [6:0], step sel2 [5:0], step sel3 [1:0]: Sets the step size of the current-output DACs in each on-chip stimulator.
 */
uint16_t rhs2116_regValue_STIM_CUR_STEP(uint8_t stepSel1, uint8_t stepSel2, uint8_t stepSel3)
{
	// Ensure the values fit in their respective fields
	stepSel1 &= 0x7F; // Mask to 7 bits
	stepSel2 &= 0x3F; // Mask to 6 bits
	stepSel3 &= 0x3;  // Mask to 2 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (stepSel3 << 13) | (stepSel2 << 7) | stepSel1;

	return command;
}

/*
 * Configures Register 35: Stimulation Bias Voltages
 * stim Pbias [3:0] and stim Nbias [3:0]: Configures internal bias voltages for the stimulator circuits.
 */
uint16_t rhs2116_regValue_STIM_BIAS_VOLTS(uint8_t stimPbias, uint8_t stimNbias)
{
	// Ensure the values fit in their respective fields
	stimPbias &= 0xF; // Mask to 4 bits
	stimNbias &= 0xF; // Mask to 4 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (stimPbias << 4) | stimNbias;

	return command;
}

/*
 * Configures Register 36: Current-Limited Charge Recovery Target Voltage
 * charge recovery DAC [7:0]: Sets the output voltage of the DAC for current-limited charge recovery circuits.
 */
uint16_t rhs2116_regValue_CHRG_REC_VOLTS(uint8_t chargeRecoveryDac)
{
	// Ensure the value fits in its respective field
	chargeRecoveryDac &= 0xFF; // Mask to 8 bits

	// Construct the command by placing the value in the correct position
	uint16_t command = chargeRecoveryDac;

	return command;
}

/*
 * Configures Register 37: Charge Recovery Current Limit
 * Imax sel1 [6:0], Imax sel2 [5:0], Imax sel3 [1:0]: Sets the maximum current for the current-limited charge recovery circuit.
 */
uint16_t rhs2116_regValue_CHRG_REC_CUR_LIM(uint8_t imaxSel1, uint8_t imaxSel2, uint8_t imaxSel3)
{
	// Ensure the values fit in their respective fields
	imaxSel1 &= 0x7F; // Mask to 7 bits
	imaxSel2 &= 0x3F; // Mask to 6 bits
	imaxSel3 &= 0x3;  // Mask to 2 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (imaxSel3 << 13) | (imaxSel2 << 7) | imaxSel1;

	return command;
}

/*
 * Configures Register 38: Individual DC Amplifier Power
 * DC amp power [15:0]: Powers down individual DC-coupled low-gain amplifiers when set to 0 (not recommended due to a hardware bug).
 */
uint16_t rhs2116_regValue_DC_AMP_PWR(uint16_t dcAmpPower)
{
	// Ensure the value fits in its respective field
	dcAmpPower &= 0xFFFF; // Mask to 16 bits

	// The command is the same as the dcAmpPower value
	uint16_t command = dcAmpPower;

	return command;
}

/*
 * Configures an individual channel's negative stimulation current magnitude and trim (TRIGGERED REGISTERS)
 * channel: The stimulator channel number (0-15).
 * negativeCurrentMagnitude: The magnitude of the negative current for the specified channel.
 * negativeCurrentTrim: The trim value for the negative stimulation current for the specified channel.
 */
void rhs2116_regValue_NEG_CUR_MAG_X(uint8_t channel, uint8_t negativeCurrentMagnitude, uint8_t negativeCurrentTrim)
{
	// Ensure the values fit in their respective fields
	negativeCurrentMagnitude &= 0xFF; // Mask to 8 bits
	negativeCurrentTrim &= 0xFF;	  // Mask to 8 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (negativeCurrentTrim << 8) | negativeCurrentMagnitude;
}

/*
 * Configures an individual channel's positive stimulation current magnitude and trim (TRIGGERED REGISTERS)
 * channel: The stimulator channel number (0-15).
 * positiveCurrentMagnitude: The magnitude of the positive current for the specified channel.
 * positiveCurrentTrim: The trim value for the positive stimulation current for the specified channel.
 */
void rhs2116_regValue_POS_CUR_MAG_X(uint8_t channel, uint8_t positiveCurrentMagnitude, uint8_t positiveCurrentTrim)
{
	// Ensure the values fit in their respective fields
	positiveCurrentMagnitude &= 0xFF; // Mask to 8 bits
	positiveCurrentTrim &= 0xFF;	  // Mask to 8 bits

	// Construct the command by shifting and combining the fields
	uint16_t command = (positiveCurrentTrim << 8) | positiveCurrentMagnitude;
}
