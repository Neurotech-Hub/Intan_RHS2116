/***************************************************************************//**
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

void Rhs2116_Init(SPIDRV_Handle_t spiHandle) {
	rhs2116_context.spiHandle = spiHandle;
	// Additional initialization code for RHS2116 can go here
}

// Flag to signal that transfer is complete
static volatile bool transfer_complete = false;

// Callback fired when data is transmitted
void transfer_callback(SPIDRV_HandleData_t *handle, Ecode_t transfer_status,
		int items_transferred) {
	(void) &handle;
	(void) items_transferred;

	// Post semaphore to signal to application
	// task that transfer is successful
	if (transfer_status == ECODE_EMDRV_SPIDRV_OK) {
		transfer_complete = true;
	}
}

void rhs2116_init(SPIDRV_Handle_t spiHandle) {
	rhs2116_context.spiHandle = spiHandle;
}

uint16_t do_transfer(void) {
	Ecode_t ecode;
	transfer_complete = false;
	ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &tx_buffer, &rx_buffer,
			sizeof(tx_buffer), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;

	//dummy cycles
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
	uint16_t receivedData = ((rx_buffer >> 8) & 0xFF00)
			| ((rx_buffer >> 24) & 0xFF);

	return receivedData;
}

bool rhs2116_writeRegister(uint8_t regAddress, uint16_t regValue, bool uFlag,
bool mFlag) {
	tx_buffer = 0;

	// Split regValue into MSB and LSB
	uint8_t dataMSB = (regValue >> 8) & 0xFF; // Extract MSB of the data
	uint8_t dataLSB = regValue & 0xFF;       // Extract LSB of the data

	// Construct the LSB byte of the command with the specified structure
	uint8_t lsbCommand = 0x80; // Bit 7 set to 1, bits 6-0 set to 0 as base
	if (uFlag) {
		lsbCommand |= 0x20; // Set U flag (bit 5)
	}
	if (mFlag) {
		lsbCommand |= 0x10; // Set M flag (bit 4)
	}

	// Assemble the command with dataLSB in the most significant position
	tx_buffer = ((uint32_t) dataLSB << 24) | ((uint32_t) dataMSB << 16)
			| ((uint32_t) regAddress << 8) | lsbCommand;

	uint16_t receivedData = do_transfer();

	// Check if the received value matches the sent value
	if (receivedData == regValue) {
		return true; // Data integrity check passed
	}
	return false; // Data integrity check failed
}

uint16_t rhs2116_readRegister(uint8_t regAddress, bool uFlag, bool mFlag) {
	tx_buffer = 0;

	uint8_t lsbCommand = 0xC0; // Bits 7 and 6 set to 1, bits 5-0 set to 0 as base
	if (uFlag) {
		lsbCommand |= 0x20; // Set U flag (bit 5)
	}
	if (mFlag) {
		lsbCommand |= 0x10; // Set M flag (bit 4)
	}

	tx_buffer = ((uint32_t) regAddress << 8) | (uint32_t) lsbCommand;
	uint16_t receivedData = do_transfer();
	return receivedData;
}

void rhs2116_clear(void) {
	// Clear the buffers
	tx_buffer = 0;
	rx_buffer = 0;

	tx_buffer = 0x0000006A;
	do_transfer();
}

bool rhs2116_checkId(void) {
	uint16_t chipId = rhs2116_readRegister(RHS_CHIP_ID, false, false);
	if (chipId == CHIP_ID) {
		return true;
	} else {
		return false;
	}
}

//void spidrv_app_process_action(void)
//{
//  Ecode_t ecode;
//
//  // Delay to allow slave to start
//  sl_sleeptimer_delay_millisecond(10000);
//
//  sprintf(tx_buffer, "ping %03d", counter);
//  counter++;
//  printf("Sending %s to slave...\r\n", tx_buffer);
//
//  transfer_complete = false;
//
//  // Non-blocking data transfer to slave. When complete, rx buffer
//  // will be filled.
//  ecode = SPIDRV_MTransfer(SPI_HANDLE, tx_buffer, rx_buffer, APP_BUFFER_SIZE, transfer_callback);
//  EFM_ASSERT(ecode == ECODE_OK);
//
//  // wait for transfer to complete
//  while (!transfer_complete) ;
//
//  printf("Got message from slave: %s\r\n", rx_buffer);
//}
