/***************************************************************************//**
 * @file rhs2116.h
 * @brief Intan RHS2116 library
 ******************************************************************************/

#ifndef RHS_2116_H
#define RHS_2116_H

#include "spidrv.h"

#define CHIP_ID					0x20

#define RHS_CLEAR				0x6A
#define RHS_SUPPS_BIASCURR		0
#define RHS_OUTFMT_DSP_AUXDIO	1
#define RHS_IMPCHK_CTRL			2
#define RHS_IMPCHK_DAC			3
#define RHS_AMP_BWDTH_1			4
#define RHS_AMP_BWDTH_2			5
#define RHS_AMP_BWDTH_3			6
#define RHS_AMP_BWDTH_4			7
#define RHS_ACAMP_PWR			8
#define RHS_AMP_FSTSETL			10
#define RHS_AMP_LCUTOFF			12
#define RHS_STIM_EN_A			32
#define RHS_STIM_EN_B			33
#define RHS_STIM_CUR_STEP		34
#define RHS_STIM_BIAS_VOLTS		35
#define RHS_CHRG_REC_VOLTS		36
#define RHS_CHRG_REC_CUR_LIM	37
#define RHS_DC_AMP_PWR			38
#define RHS_COMPL_MON			40
#define RHS_STIM_ON				42
#define RHS_STIM_POL			44
#define RHS_CHRG_RECOVER		46
#define RHS_CUR_LMT_CHRG_REC	48
#define RHS_FAULT_CUR_DET		50
#define RHS_NEG_CUR_MAG_0		64
#define RHS_NEG_CUR_MAG_1		65
#define RHS_NEG_CUR_MAG_2		66
#define RHS_NEG_CUR_MAG_3		67
#define RHS_NEG_CUR_MAG_4		68
#define RHS_NEG_CUR_MAG_5		69
#define RHS_NEG_CUR_MAG_6		70
#define RHS_NEG_CUR_MAG_7		71
#define RHS_NEG_CUR_MAG_8		72
#define RHS_NEG_CUR_MAG_9		73
#define RHS_NEG_CUR_MAG_10		74
#define RHS_NEG_CUR_MAG_11		75
#define RHS_NEG_CUR_MAG_12		76
#define RHS_NEG_CUR_MAG_13		77
#define RHS_NEG_CUR_MAG_14		78
#define RHS_NEG_CUR_MAG_15		79
#define RHS_POS_CUR_MAG_0		96
#define RHS_POS_CUR_MAG_1		97
#define RHS_POS_CUR_MAG_2		98
#define RHS_POS_CUR_MAG_3		99
#define RHS_POS_CUR_MAG_4		100
#define RHS_POS_CUR_MAG_5		101
#define RHS_POS_CUR_MAG_6		102
#define RHS_POS_CUR_MAG_7		103
#define RHS_POS_CUR_MAG_8		104
#define RHS_POS_CUR_MAG_9		105
#define RHS_POS_CUR_MAG_10		106
#define RHS_POS_CUR_MAG_11		107
#define RHS_POS_CUR_MAG_12		108
#define RHS_POS_CUR_MAG_13		109
#define RHS_POS_CUR_MAG_14		110
#define RHS_POS_CUR_MAG_15		111
#define RHS_COMP_IN				251
#define RHS_COMP_TA				252
#define RHS_COMP_N				253
#define RHS_NCH					254
#define RHS_CHIP_ID				255 // RHS2116 = 32 (0x20)

typedef struct {
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

#endif  // RHS_2116_H
