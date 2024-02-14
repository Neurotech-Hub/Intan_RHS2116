/***************************************************************************/ /**
																			   * @file rhs2116.h
																			   * @brief Intan RHS2116 library
																			   ******************************************************************************/

#ifndef RHS2116_H
#define RHS2116_H

#include "spidrv.h"

#define CHIP_ID 0x20
#define RHS_CLEAR 0x6A

#define RHS_SUPPS_BIASCURR 0
#define RHS_OUTFMT_DSP_AUXDIO 1
#define RHS_IMPCHK_CTRL 2
#define RHS_IMPCHK_DAC 3
#define RHS_RH1_CUTOFF 4
#define RHS_RH2_CUTOFF 5
#define RHS_ARL_A_CUTOFF 6
#define RHS_ARL_B_CUTOFF 7
#define RHS_ACAMP_PWR 8
#define RHS_AMP_FSTSETL 10
#define RHS_AMP_LCUTOFF 12
#define RHS_STIM_EN_A 32
#define RHS_STIM_EN_B 33
#define RHS_STIM_CUR_STEP 34
#define RHS_STIM_BIAS_VOLTS 35
#define RHS_CHRG_REC_VOLTS 36
#define RHS_CHRG_REC_CUR_LIM 37
#define RHS_DC_AMP_PWR 38
#define RHS_COMPL_MON 40
#define RHS_STIM_ON 42
#define RHS_STIM_POL 44
#define RHS_CHRG_RECOVER 46
#define RHS_CUR_LMT_CHRG_REC 48
#define RHS_FAULT_CUR_DET 50
#define RHS_NEG_CUR_MAG_0 64
#define RHS_NEG_CUR_MAG_1 65
#define RHS_NEG_CUR_MAG_2 66
#define RHS_NEG_CUR_MAG_3 67
#define RHS_NEG_CUR_MAG_4 68
#define RHS_NEG_CUR_MAG_5 69
#define RHS_NEG_CUR_MAG_6 70
#define RHS_NEG_CUR_MAG_7 71
#define RHS_NEG_CUR_MAG_8 72
#define RHS_NEG_CUR_MAG_9 73
#define RHS_NEG_CUR_MAG_10 74
#define RHS_NEG_CUR_MAG_11 75
#define RHS_NEG_CUR_MAG_12 76
#define RHS_NEG_CUR_MAG_13 77
#define RHS_NEG_CUR_MAG_14 78
#define RHS_NEG_CUR_MAG_15 79
#define RHS_POS_CUR_MAG_0 96
#define RHS_POS_CUR_MAG_1 97
#define RHS_POS_CUR_MAG_2 98
#define RHS_POS_CUR_MAG_3 99
#define RHS_POS_CUR_MAG_4 100
#define RHS_POS_CUR_MAG_5 101
#define RHS_POS_CUR_MAG_6 102
#define RHS_POS_CUR_MAG_7 103
#define RHS_POS_CUR_MAG_8 104
#define RHS_POS_CUR_MAG_9 105
#define RHS_POS_CUR_MAG_10 106
#define RHS_POS_CUR_MAG_11 107
#define RHS_POS_CUR_MAG_12 108
#define RHS_POS_CUR_MAG_13 109
#define RHS_POS_CUR_MAG_14 110
#define RHS_POS_CUR_MAG_15 111
#define RHS_COMP_IN 251
#define RHS_COMP_TA 252
#define RHS_COMP_N 253
#define RHS_NCH 254
#define RHS_CHIP_ID 255 // RHS2116 = 32 (0x20)

typedef struct
{
	SPIDRV_Handle_t spiHandle;
} Rhs2116_Context_t;

void rhs2116_init(SPIDRV_Handle_t spiHandle);
void transfer_callback(SPIDRV_HandleData_t *handle, Ecode_t transfer_status,
					   int items_transferred);
uint16_t do_transer(void);
bool rhs2116_writeRegister(uint8_t regAddress, uint16_t regValue, bool uFlag, bool mFlag);
uint16_t rhs2116_readRegister(uint8_t regAddress, bool uFlag, bool mFlag);
void rhs2116_clear(void);
bool rhs2116_checkId(void);
uint16_t rhs2116_convert(uint8_t channel, bool uFlag, bool mFlag, bool dFlag, bool hFlag);
bool rhs2116_SUPPS_BIASCURR(uint8_t adcBufferBias, uint8_t muxBias);
bool rhs2116_OUTFMT_DSP_AUXDIO(uint8_t dspCutoffFreq, bool dspEn, bool absMode, bool twosComp, bool weakMiso, bool digout1HiZ, bool digout1, bool digout2HiZ, bool digout2, bool digoutOD);
bool rhs2116_IMPCHK_CTRL(uint8_t zcheckSelect, bool zcheckDacPower,
bool zcheckLoad, uint8_t zcheckScale, bool zcheckEn);
bool rhs2116_IMPCHK_DAC(uint8_t zcheckDac);
bool rhs2116_RH1_CUTOFF(uint8_t rh1Sel1, uint8_t rh1Sel2);
bool rhs2116_RH2_CUTOFF(uint8_t rh2Sel1, uint8_t rh2Sel2);
bool rhs2116_RL_A_CUTOFF(uint8_t rlASel1, uint8_t rlASel2, bool rlASel3);
bool rhs2116_RL_B_CUTOFF(uint8_t rlBSel1, uint8_t rlBSel2, bool rlBSel3);
bool rhs2116_ACAMP_PWR(uint16_t acAmpPower);
bool rhs2116_AMP_FSTSETL(uint16_t ampFastSettle);
bool rhs2116_AMP_LCUTOFF(uint16_t ampFLSelect);
bool rhs2116_STIM_EN_A(uint16_t stimEnableA);
bool rhs2116_STIM_EN_B(uint16_t stimEnableB);
bool rhs2116_STIM_CUR_STEP(uint8_t stepSel1, uint8_t stepSel2, uint8_t stepSel3);
bool rhs2116_STIM_BIAS_VOLTS(uint8_t stimPbias, uint8_t stimNbias);
bool rhs2116_CHRG_REC_VOLTS(uint8_t chargeRecoveryDac);
bool rhs2116_CHRG_REC_CUR_LIM(uint8_t imaxSel1, uint8_t imaxSel2, uint8_t imaxSel3);
bool rhs2116_DC_AMP_PWR(uint16_t dcAmpPower);
bool rhs2116_STIM_ON(uint16_t stimOn);
bool rhs2116_STIM_POL(uint16_t stimPol);
bool rhs2116_CHRG_RECOVER(uint16_t chargeRecoverySwitch);
bool rhs2116_CUR_LMT_CHRG_REC(uint16_t clChargeRecoveryEnable);
bool rhs2116_NEG_CUR_MAG_X(uint8_t channel, uint8_t negativeCurrentMagnitude, uint8_t negativeCurrentTrim);
bool rhs2116_POS_CUR_MAG_X(uint8_t channel, uint8_t positiveCurrentMagnitude, uint8_t positiveCurrentTrim);

#endif // RHS2116_H
