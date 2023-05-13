#include "bms.h"

#ifdef BMS_DEBUG
#include <stdarg.h>
#include <stdio.h>
#endif

// List of errors, that if occur, the device will not be revived
const uint16_t bms_revive_blacklist[] = { BMS_BIT_OVP, BMS_BIT_IMBALANCE, BMS_BIT_PREQF, BMS_BIT_PMFAIL };

__weak void bms_print(char *str)
{
	return;
}

__weak void bms_log(char *fmt, ...)
{
#ifdef BMS_DEBUG
    va_list args;
    va_start(args, fmt);
    sprintf(fmt, args);
    va_end(args);
#endif
}

__weak void bms_delay(uint32_t msecs)
{
	return;
}

__weak bms_error_t bms_reg_write(uint16_t reg, uint8_t * data, uint16_t len)
{
	return BMS_I2C_MEM_WRITE_ERROR;
}

__weak bms_error_t bms_reg_read(uint16_t reg, uint8_t * data, uint16_t len)
{
	return BMS_I2C_MEM_READ_ERROR;
}

bms_error_t bms_full_reset()
{
	return BMS_RESET_ERROR;
}

bms_error_t bms_apply_configuration()
{
#ifdef BMS_DEBUG
	bms_log("bms_apply_configuration: Configuration is not applied in runtime SW. This is completed in the factory.\n");
#endif

	return BMS_OK;
}

bms_error_t bms_bat_okay()
{
	uint16_t data = 0;

	// 1. Check for permanent fault. If so leave it and return the result.

	// Read the battery voltage data
	uint8_t status = bms_reg_read(BMS_REG_NBATTSTATUS, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_bat_okay: Error reading nBattStatus register.\n");
#endif
		return status;
	}

	// See if the perm fail bit is set
	if(data & BMS_BIT_PERMFAIL) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_bat_okay: Permanent fail detected, this device is irrepairable.\n");
#endif
		return BMS_PERM_FAIL_ERROR;
	}

	// 2. Read the status register to see if there has been any protection events
	status = bms_reg_read(BMS_REG_STATUS, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_bat_okay: Error reading status register.\n");
#endif
		return status;
	}

	// See if the PA (protection alert) bit is set
	if(data & BMS_BIT_PROTALRT)
	{
#ifdef BMS_DEBUG
		bms_log("bms_bat_okay: Protection alert bit set, looking to see if it can be corrected.\n");
#endif

		// 2.1 Get the historic protection alert data
		status = bms_reg_read(BMS_REG_PROTALRT, (uint8_t*)&data, 2);

		if(status != BMS_OK) 
		{
#ifdef BMS_DEBUG
			bms_log("bms_bat_okay: Error reading protection alert register register.\n");
#endif
			return status;
		}

		// Go through the blacklist in case we should not revive
		for(uint32_t i = 0; i < (sizeof(bms_revive_blacklist) / sizeof(bms_revive_blacklist[0])); i++)
		{
			if(data & bms_revive_blacklist[i])
			{
				// If there is an irrepairable fault
#ifdef BMS_DEBUG
				bms_log("bms_bat_okay: Cannot recover from %u fault in PROTALRT reg.\n", bms_revive_blacklist[i]);
#endif				
				return BMS_CANNOT_REC;
			}
		}

#ifdef BMS_DEBUG
		bms_log("bms_bat_okay: Found no issues that cannot be repaired. Fully resetting the BMS to repair.\n");
#endif	

		// 2.2 Full reset
		status = bms_full_reset();

		if(status != BMS_OK) 
		{
#ifdef BMS_DEBUG
			bms_log("bms_bat_okay: Could not reset the BMS.\n");
#endif
			return status;
		}
	}

#ifdef BMS_DEBUG
			bms_log("bms_bat_okay: BMS is okay.\n");
#endif

	return BMS_OK;
}

bms_error_t bms_get_voltage(float * voltage)
{
	uint16_t data = 0;

	// Read the battery voltage data
	uint8_t status = bms_reg_read(BMS_REG_VBAT, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_voltage: Error reading raw voltage data from the BMS.\n");
#endif
		return status;
	}

	// Convert
	*voltage = (float)data * 0.3125e-3;

	return BMS_OK;
}

bms_error_t bms_get_current(float * current)
{
	int16_t data = 0;

	// Read the resistor voltage data
	uint8_t status = bms_reg_read(BMS_REG_CBAT, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_current: Error reading resistor voltage data from the BMS.\n");
#endif
		return status;
	}

	// Convert
	*current = (float)data * 1.5625e-3;

	return BMS_OK;
}

bms_error_t bms_get_soc(float * soc)
{
	uint16_t data = 0;

	// Read the resistor voltage data
	uint8_t status = bms_reg_read(BMS_REG_REPCAP, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_soc: Error reading resistor voltage data from the BMS.\n");
#endif
		return status;
	}

	// Convert
	*soc = (float)data * 5e-3;

	return BMS_OK;
}

bms_error_t bms_get_full_capacity(float * full_cap)
{
	uint16_t data = 0;

	// Read the resistor voltage data
	uint8_t status = bms_reg_read(BMS_REG_FULLCAP, (uint8_t*)&data, 2);

	if(status != BMS_OK) 
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_full_capacity: Error reading resistor voltage data from the BMS.\n");
#endif
		return status;
	}

	// Convert
	*full_cap = (float)data * 5e-3;

	return BMS_OK;
}

bms_error_t bms_get_nv_writes_remaining(uint8_t * count)
{
	uint8_t status = BMS_OK;
	uint16_t value = 0;

	// Write 0x0000 to the CommStat register (0x61) two times in a row to unlock write protection.
	value = 0;
	status = bms_reg_write(BMS_REG_COMMSTAT, (uint8_t*)&value, 2);
	status = bms_reg_write(BMS_REG_COMMSTAT, (uint8_t*)&value, 2);
	
	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_updates_remaining: Error writing 0x0000 to BMS_REG_COMMSTAT.\n");
#endif		
		return status;
	}

	// Write 0xE29B to the Command register 0x060
	value = 0xE29B;
	status = bms_reg_write(BMS_REG_CMDREG, (uint8_t*)&value, 2);
	
	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_updates_remaining: Error writing 0xE29B to BMS_REG_CMDREG for full reset.\n");
#endif		
		return status;
	}

	// Wait atleast tRECALL (5ms)
	bms_delay(10);

	// Read reg for the write cycle information
	status = bms_reg_read(0x01FD, (uint8_t*)&value, 2);
	
	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_updates_remaining: Error reading BMS_REG_CNFG2.\n");
#endif		
		return status;
	}

	// Count how many 0s there are at the MSB, this how many writes are remaining. 
	for(uint8_t i = 0; i < 8; i++)
	{
		if(value & (0x8000 >> i)) 
		{
			*count = i;
			break;
		}
	}

	// Write 0x00F9 to the CommStat register (0x61) two times in a row to lock write protection.
	value = 0x00F9;
	status = bms_reg_write(BMS_REG_COMMSTAT, (uint8_t*)&value, 2);
	status = bms_reg_write(BMS_REG_COMMSTAT, (uint8_t*)&value, 2);
	
	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_updates_remaining: Error writing 0x00F9 to BMS_REG_COMMSTAT.\n");
#endif		
		return status;
	}

	return BMS_OK;
}

bms_error_t bms_get_stats(bms_stats * stats)
{
	uint8_t status = BMS_OK;
	
	// Get voltage
	status = bms_get_voltage(&stats->volts);

	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_stats: Reading voltage not okay, error code = %u.\n", status);
#endif			
		return status;
	}

	// Get current
	status = bms_get_current(&stats->amps);

	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_stats: Reading current not okay, error code = %u.\n", status);
#endif			
		return status;
	}

	// Get SoC
	status = bms_get_soc(&stats->soc);

	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_stats: Reading SoC not okay, error code = %u.\n", status);
#endif			
		return status;
	}

	// Get Capacity
	status = bms_get_full_capacity(&stats->cap);

	if(status != BMS_OK)
	{
#ifdef BMS_DEBUG
		bms_log("bms_get_stats: Reading capacity not okay, error code = %u.\n", status);
#endif			
		return status;
	}

	return status;	
}

bms_error_t bms_clear_permfail()
{
	return BMS_PERM_FAIL_RESET_ERROR;
}


