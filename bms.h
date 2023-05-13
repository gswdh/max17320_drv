#ifndef _BMS_H_
#define _BMS_H_

#include <stdint.h>

#define BMS_I2C_L_ADDR			(0x6C)
#define BMS_I2C_H_ADDR			(0x16)

#define BMS_I2C_ADDR(reg)		((reg > 0xFF) ? BMS_I2C_H_ADDR : BMS_I2C_L_ADDR)

#define T_BMS_POR_TIMEOUT_MS    10000

#define BMS_REG_STATUS          (0x0000)
#define BMS_REG_PROTSTATUS      (0x00D9)
#define BMS_REG_PROTALRT        (0x00AF)
#define BMS_REG_NBATTSTATUS     (0x01A8)
#define BMS_REG_VBAT			(0x00DA)
#define BMS_REG_CBAT			(0x001C)
#define BMS_REG_COMMSTAT		(0x0061)
#define BMS_REG_CMDREG			(0x0060)
#define BMS_REG_CNFG2           (0x00AB)
#define BMS_REG_REPCAP          (0x0005)
#define BMS_REG_FULLCAP         (0x0035)

#define BMS_BIT_PERMFAIL        (0x8000)
#define BMS_BIT_PROTALRT        (0x8000)

#define BMS_BIT_CHGWDT          (0x8000)
#define BMS_BIT_TOOHOTC         (0x4000)
#define BMS_BIT_FULL            (0x2000)
#define BMS_BIT_TOOCOLDC        (0x1000)
#define BMS_BIT_OVP             (0x0800)
#define BMS_BIT_OCCP            (0x0400)
#define BMS_BIT_QOVFLW          (0x0200)
#define BMS_BIT_PREQF           (0x0100)
#define BMS_BIT_IMBALANCE       (0x0080)
#define BMS_BIT_PMFAIL          (0x0040)
#define BMS_BIT_DIEHOT          (0x0020)
#define BMS_BIT_TOOHOTD         (0x0010)
#define BMS_BIT_UVP             (0x0008)
#define BMS_BIT_ODCP            (0x0004)
#define BMS_BIT_RESDFAULT       (0x0002)
#define BMS_BIT_LDET            (0x0001)

// A results struct for the ACT ADC
struct bms_stats_tag;
typedef struct bms_stats_tag
{
    float volts;
    float amps;
    float soc;
    float cap;
}
bms_stats_t;

// Error handling
typedef enum bms_error_t {  BMS_OK, 
                            BMS_RESET_ERROR, 
                            BMS_I2C_MEM_WRITE_ERROR, 
                            BMS_I2C_MEM_READ_ERROR,
                            BMS_CANNOT_REC_ERROR,
                            BMS_PERM_FAIL_ERROR,
                            BMS_PERM_FAIL_RESET_ERROR };

/*
 *  Function: bms_print
 *  ----------------------------
 *  Writes data into the registers of the MAX17320 BMS
 *
 *  reg: memory start position in the BMS
 *  data: pointer to the data being written into the BMS
 *  len: length of the data to be written in bytes
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
void bms_print(char *str);

/*
 *  Function: bms_log
 *  ----------------------------
 *  Writes data into the registers of the MAX17320 BMS
 *
 *  reg: memory start position in the BMS
 *  data: pointer to the data being written into the BMS
 *  len: length of the data to be written in bytes
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
void bms_log(char *fmt, ...);

/*
 *  Function: bms_delay
 *  ----------------------------
 *  Writes data into the registers of the MAX17320 BMS
 *
 *  reg: memory start position in the BMS
 *  data: pointer to the data being written into the BMS
 *  len: length of the data to be written in bytes
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
void bms_delay(uint32_t msecs);

/*
 *  Function: bms_reg_write
 *  ----------------------------
 *  Writes data into the registers of the MAX17320 BMS
 *
 *  reg: memory start position in the BMS
 *  data: pointer to the data being written into the BMS
 *  len: length of the data to be written in bytes
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
bms_error_t bms_reg_write(uint16_t reg, uint8_t * data, uint16_t len);

/*
 *  Function: bms_reg_read
 *  ----------------------------
 *  Writes data into the registers of the MAX17320 BMS
 *
 *  reg: memory start position in the BMS
 *  data: pointer to the data being written into the BMS
 *  len: length of the data to be written in bytes
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
bms_error_t bms_reg_read(uint16_t reg, uint8_t * data, uint16_t len);

/*
 *  Function: bms_full_reset
 *  ----------------------------
 *  Will fully reset the BMS as if the power is cycled (cells removed).
 *  Useful when removing faults.
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
bms_error_t bms_full_reset();

/*
 * 	Function: bms_apply_configuration
 * 	----------------------------
 *  Will write the non volatile configuration into the BMS.
 *
 *  returns: an error code from bms_error_t enum based on success 
 */
bms_error_t bms_apply_configuration();

/*
 * 	Function: bms_bat_okay
 * 	----------------------------
 *  Reads data from the BMS and compares to a parameterisable list of
 * 	thresholds to determine whether the battery is fit for use like 
 * 	heating and OTG.
 *
 *  returns: an error code from bms_error_t enum based on battery
 *	status 
 */
bms_error_t bms_bat_okay();

/*
 * 	Function: bms_get_voltage
 * 	----------------------------
 *  Reads the current battery voltage
 *
 *  voltage: pointer to the memory for the voltage to be stored (V)
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_voltage(float * voltage);

/*
 * 	Function: bms_get_current
 * 	----------------------------
 *  Reads the current battery current consumption
 *
 *  current: pointer to the memory for the current to be stored (A)
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_current(float * current);

/*
 * 	Function: bms_get_soc
 * 	----------------------------
 *  Reads the current battery current state of charge
 *
 *  soc: pointer to the memory for the SoC to be stored (Ah)
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_soc(float * soc);

/*
 *  Function: bms_get_full_capacity
 *  ----------------------------
 *  Reads the current calculated maximum capacity of the battery
 *
 *  full_cap: pointer to the memory for the capacity to be stored (Ah)
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_full_capacity(float * full_cap);

/*
 *  Function: bms_get_stats
 *  ----------------------------
 *  Reads the current statistics in the BMS
 *
 *  stats: pointer to the memory for the statistics to be stored (Ah)
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_stats(bms_stats * stats);

/*
 * 	Function: bms_get_nv_writes_remaining
 * 	----------------------------
 *  Reads the current battery current state of charge
 *
 *  soc: pointer to the memory for the SoC to be stored
 *
 *  returns: an error code from bms_error_t enum based on success
 */
bms_error_t bms_get_nv_writes_remaining(uint8_t * count);

#endif