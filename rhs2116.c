/***************************************************************************//**
 * @file rhs2116.c
 * @brief Intan RHS2116 library
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "rhs2116.h"
#include "spidrv.h"

static Rhs2116_Context_t rhs2116_context;

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

void rhs2116_writeRegister(uint8_t regAddress, uint32_t regValue) {
	uint32_t command = (regAddress << 24) | regValue;
	transfer_complete = false; // Reset the completion flag before starting the transfer

	// Initiate the transfer, reuse regValue (cannot be NULL)
	Ecode_t ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &command,
			&regValue, sizeof(command), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;
}

uint32_t rhs2116_readRegister(uint8_t regAddress) {
	uint32_t command = (0x80000000 | (regAddress << 24));
	uint32_t regValue = 0;
	transfer_complete = false; // Reset the completion flag before starting the transfer

	// Initiate the transfer to send the command
	Ecode_t ecode = SPIDRV_MTransfer(rhs2116_context.spiHandle, &command,
			&regValue, sizeof(command), transfer_callback);
	EFM_ASSERT(ecode == ECODE_EMDRV_SPIDRV_OK);

	// Wait for the transfer to complete
	while (!transfer_complete)
		;

	return regValue;
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
