
#include <stdio.h>
#include <string.h>
#include "kp6910xx.h"
#include "batt_model.h"

static KP6910XX_T *pkp6910xx;


int32_t kp6910xx_intf_register(KP6910XX_T *kp6910xx, I2C_READ_WRITE_T *pi2c_intf, FW_STRUCT_T *pfw)
{
    if(pi2c_intf == NULL )
        return 1;

    pkp6910xx = kp6910xx;
    pkp6910xx->pi2c_intf = pi2c_intf;
    pkp6910xx->pfwbaseaddr = pfw;

    return 0;
}

int32_t kp6910xx_set_tabledata(KP6910XX_T *chip, uint16_t addr, uint8_t *pdata, uint16_t len )
{
    uint16_t  uRWAddr = addr;
    uint8_t 	uRet = 0;
    uint8_t   uDataBuff[36];
    uint16_t  uRemain = len;
    uint16_t  uPackLen = 0;
    uint16_t  uOffset = 0;

    if(addr > TABLE_END_ADDR || addr < TABLE_START_ADDR )
        return 1;


    while(uRemain)
    {
        uRWAddr += uPackLen;

        uDataBuff[0] = uRWAddr & 0xff;
        uDataBuff[1] = (uRWAddr >> 8) & 0xff;
        uPackLen = (uRemain > SBS44_MAX_VALID_DATA_SIZE) ? SBS44_MAX_VALID_DATA_SIZE : uRemain;
        memcpy(uDataBuff + 2, pdata + uOffset, uPackLen);

        if ((uRet = chip->pi2c_intf->i2c_write_func(SBS44_MFGBLKACCESS, uDataBuff, uPackLen + 2)) != 0)
        {
            chip->pi2c_intf->debug_func("Table Data: %x  Send Failed\r\n", uRWAddr );
            break;
        }
        chip->pi2c_intf->delay_func(10);

        uRemain -= uPackLen;
        uOffset += uPackLen;
    }

    return uRet;
}

int32_t kp6910xx_get_tabledata(KP6910XX_T *chip, uint16_t addr, uint8_t *pdata, uint16_t len )
{
    uint16_t  uRWAddr = addr;
    uint8_t   uRet = 0;
    uint8_t   uDataBuff[36];
    uint16_t  uRemain = len;
    uint16_t  uPackLen = 0;
    uint16_t  uOffset = 0;
	
    if(addr > TABLE_END_ADDR || addr < TABLE_START_ADDR)
        return 1;
	
    while(uRemain)
    {
        uRWAddr += uPackLen;

        uDataBuff[0] = uRWAddr & 0xff;
        uDataBuff[1] = (uRWAddr >> 8) & 0xff;
        uPackLen = (uRemain > SBS44_MAX_VALID_DATA_SIZE) ? SBS44_MAX_VALID_DATA_SIZE : uRemain;

		if ((uRet = chip->pi2c_intf->i2c_write_func(SBS44_MFGBLKACCESS, uDataBuff, 2)) != 0)
		{
			chip->pi2c_intf->debug_func("Table Addr: %x  Send Failed\r\n", uRWAddr );
			return  uRet;
		}
		chip->pi2c_intf->delay_func(1);
		if ((uRet = chip->pi2c_intf->i2c_read_func(SBS44_MFGBLKACCESS, uDataBuff, uPackLen + 2)) != 0)
		{
			chip->pi2c_intf->debug_func("Table Data: %x  Read Failed\r\n", uRWAddr );
			return  uRet;
		}
		
		memcpy(pdata + uOffset, uDataBuff + 2, uPackLen);
	
        chip->pi2c_intf->delay_func(10);

        uRemain -= uPackLen;
        uOffset += uPackLen;
    }
	
    return 0;
}

uint8_t kp6910xx_set_paramdata(KP6910XX_T *chip, uint16_t addr, uint16_t cfg_data)
{
    uint8_t uRet = 0;
    uint8_t buffer[4];
    uint8_t indx = 0;
    
    buffer[indx++] = addr & 0xFF ;;
    buffer[indx++] = (addr >> 8) & 0xFF; ;
    buffer[indx++] = cfg_data & 0xFF ;
    buffer[indx++] = (cfg_data >> 8) & 0xFF;    
    
    if ((uRet = chip->pi2c_intf->i2c_write_func(SBS44_MFGBLKACCESS, buffer, 4)) != 0)
    {   chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return uRet;
    }
    
    return 0;
}

uint8_t kp6910xx_get_paramdata(KP6910XX_T *chip, uint16_t addr, uint16_t* cfg_data)
{
    uint8_t uRet = 0;
    uint8_t buffer[4];
    uint8_t indx = 0;
    
    buffer[indx++] = addr & 0xFF ;;
    buffer[indx++] = (addr >> 8) & 0xFF; ;
    
    if ((uRet = chip->pi2c_intf->i2c_write_func(SBS44_MFGBLKACCESS, buffer, 2)) != 0)
    {   chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return  uRet;
    }
    chip->pi2c_intf->delay_func(1);

    if ((uRet = chip->pi2c_intf->i2c_read_func(SBS44_MFGBLKACCESS, buffer, 4)) != 0)
    {   chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return  uRet;
    }
    memcpy(cfg_data, buffer + 2, 2);

    return 0;
}

static int32_t _kp6910xx_get_vcell(KP6910XX_T *chip)
{
    uint8_t buffer[2];
    uint32_t temp_adc = 0;

    if (0 != chip->pi2c_intf->i2c_read_func(SBS02_VCELL, buffer, 2))
    {
        chip->pi2c_intf->debug_func("SBS02_VCELL read failed\r\n");
        return 1;
    }

    temp_adc =  buffer[0] << 8 | buffer[1];
    chip->cell_volt = temp_adc * LSB_VOLT / VOLT_FACTOR ;


    return 0;
}


static int32_t _kp6910xx_get_rsoc(KP6910XX_T *chip)
{
    uint8_t buffer[2];
    uint32_t temp_adc = 0;

    if (0 != chip->pi2c_intf->i2c_read_func(SBS04_SOC, buffer, 2))
    {
        chip->pi2c_intf->debug_func("SBS04_SOC read failed\r\n");
        return 1;
    }


    temp_adc = buffer[0] * 100 +  buffer[1] * 100 / 256;
    chip->rsoc = temp_adc / 100.0;

    return 0;
}

static int32_t _kp6910xx_get_atte(KP6910XX_T *chip)
{
    uint8_t buffer[2];
    uint32_t temp = 0;

    if (0 != chip->pi2c_intf->i2c_read_func(SBS06_ALERT, buffer, 2))
    {
        chip->pi2c_intf->debug_func("SBS06_ALERT read failed\r\n");
        return 1;
    }

    temp = buffer[0] << 8 | buffer[1];
    chip->atte = temp & 0x1FFF;
    chip->alert_flag =  (temp >> 15) & 0x01;

    return 0;
}

static int32_t _kp6910xx_get_icversion(KP6910XX_T *chip)
{
    uint8_t buffer[4];

    if (0 != chip->pi2c_intf->i2c_read_func(SBS00_ICVERSION, buffer, 1))
    {
        chip->pi2c_intf->debug_func("SBS00_ICVERSION read failed\r\n");
        return 1;
    }

    chip->icversion =  buffer[0];

    return 0;

}

static int32_t _kp6910xx_get_fwversion(KP6910XX_T *chip)
{
    uint8_t buffer[4];

    if (0 != chip->pi2c_intf->i2c_read_func(SBSF0_FWVER, buffer, 4))
    {
        chip->pi2c_intf->debug_func("SBSF0_FWVER read failed\r\n");
        return 1;
    }

    chip->fwversion =  buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];

    return 0;

}

static int32_t _kp6910xx_dump_msg(KP6910XX_T *chip)
{
    int32_t  ret = 0;

    if (1) ret += _kp6910xx_get_vcell(chip);
    //chip->pi2c_intf->delay_func(1);
    
    ret += _kp6910xx_get_rsoc(chip);
    //chip->pi2c_intf->delay_func(1);

    if (0) ret += _kp6910xx_get_atte(chip);
    //chip->pi2c_intf->delay_func(1);
    
    if (0) ret += _kp6910xx_get_icversion(chip);
    //chip->pi2c_intf->delay_func(1);

    if (0) ret += _kp6910xx_get_fwversion(chip);

    //charge parameter
    //chip->pi2c_intf->debug_func("volt:%d, rsoc: %ld.%02ld, atte: %d, alert: %d\r\n", chip->cell_volt, (uint32_t)chip->rsoc, ((uint32_t)(chip->rsoc*100))%100, chip->atte, chip->alert_flag );
    //chip->pi2c_intf->debug_func("ic_ver:%x, fw_ver:%x\r\n", chip->icversion, chip->fwversion);

    chip->pi2c_intf->debug_func("volt:%d, rsoc: %ld.%02ld\r\n", chip->cell_volt, (uint32_t)chip->rsoc, ((uint32_t)(chip->rsoc*100))%100);
    return ret;
}


static int32_t _kp6910xx_clear_alert(KP6910XX_T *chip)
{

    uint8_t buffer[2];
    int32_t  uRet = 0;

    if (0 != chip->pi2c_intf->i2c_read_func(SBS06_ALERT, buffer, 2))
    {
        chip->pi2c_intf->debug_func("SBS06_ALERT read failed\r\n");
        return 1;
    }

    buffer[0] = buffer[0] & 0x1F;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS06_ALERT, buffer, 1)) != 0)
    {
        return  uRet;
    }

    return 0;
}


static int32_t _kp6910xx_set_alertthresh(KP6910XX_T *chip, uint8_t value)
{
    int32_t  uRet = 0;

    if(value > 31)
        value  = 31;

    value <<= 3;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS08_CONFIG, &value, 1)) != 0)
    {
        return  uRet;
    }

    return 0;
}

static int32_t _kp6910xx_set_wakeupcmd(KP6910XX_T *chip)
{
    int32_t  uRet = 0;
    uint8_t value = 0x00;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS0A_MODE, &value, 1)) != 0)
    {
        return  uRet;
    }

    return 0;

}

static int32_t _kp6910xx_set_sleepcmd(KP6910XX_T *chip)
{
    int32_t  uRet = 0;
    uint8_t value = 0xc0;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS0A_MODE, &value, 1)) != 0)
    {
        return  uRet;
    }

    return 0;

}

static int32_t _kp6910xx_set_qstart(KP6910XX_T *chip)
{
    int32_t  uRet = 0;
    uint8_t value = 0x30;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS0A_MODE, &value, 1)) != 0)
    {
        return  uRet;
    }

    return 0;

}

static int32_t _kp6910xx_set_resetcmd(KP6910XX_T *chip)
{
    int32_t  uRet = 0;
    uint8_t value = 0x0F;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS0A_MODE, &value, 1)) != 0)
    {
        return  uRet;
    }

    return 0;
}

static int32_t _kp6910xx_set_initsoc(KP6910XX_T *chip, int16_t init_soc)
{
    uint32_t  uIndx = 0;
    uint8_t 	 uRet = 0;
    uint8_t   buffer[2];

    buffer[uIndx++] = init_soc & 0xFF;
    buffer[uIndx++] = (init_soc >> 8) & 0xFF;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS82_EXTINITSOC, buffer, uIndx)) != 0)
    {
        return  uRet;
    }

    return 0;
}

static int32_t _kp6910xx_set_cyclecnt(KP6910XX_T *chip, uint16_t cyclecnt)
{
    uint32_t  uIndx = 0;
    uint8_t 	 uRet = 0;
    uint8_t   buffer[2];

    buffer[uIndx++] = cyclecnt & 0xFF;
    buffer[uIndx++] = (cyclecnt >> 8) & 0xFF;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS83_EXTCYCLECNT, buffer, uIndx)) != 0)
    {
        return  uRet;
    }

    return 0;
}

static int32_t _kp6910xx_set_extchargersts(KP6910XX_T *chip, int16_t ext_charger)
{
    uint32_t  uIndx = 0;
    uint8_t 	 uRet = 0;
    uint8_t   buffer[2];

    buffer[uIndx++] = ext_charger & 0xFF;
    buffer[uIndx++] = (ext_charger >> 8) & 0xFF;

    if ((uRet = pkp6910xx->pi2c_intf->i2c_write_func(SBS81_EXTCHRGER, buffer, uIndx)) != 0)
    {
        return  uRet;
    }

    return 0;
}

int32_t kp6910xx_get_dump_msg(void)
{
    return _kp6910xx_dump_msg(pkp6910xx);
}

int32_t kp6910xx_get_voltage(void)
{

    return pkp6910xx->cell_volt;
}

int32_t kp6910xx_get_temperature(void)
{
    return pkp6910xx->inttemp;
}

int32_t kp6910xx_get_atte(void)
{

    return pkp6910xx->atte;
}

int32_t kp6910xx_get_alertflag(void)
{

    return pkp6910xx->alert_flag;
}

int32_t  kp6910xx_get_rsoc(void)
{
    return pkp6910xx->rsoc;
}

int32_t kp6910xx_clear_alert(void)
{
    return _kp6910xx_clear_alert(pkp6910xx);
}

int32_t kp6910xx_set_alertthresh(uint8_t value)
{
    return _kp6910xx_set_alertthresh(pkp6910xx, value);
}

int32_t kp6910xx_set_wakeupcmd(void)
{
    return _kp6910xx_set_wakeupcmd(pkp6910xx);
}

int32_t kp6910xx_set_sleepcmd(void)
{
    return _kp6910xx_set_sleepcmd(pkp6910xx);
}

int32_t kp6910xx_set_qstart(void)
{
    return _kp6910xx_set_qstart(pkp6910xx);
}

int32_t kp6910xx_set_resetcmd(void)
{
    return _kp6910xx_set_resetcmd(pkp6910xx);
}

int32_t kp6910xx_set_initsoc(int16_t value)
{
    return _kp6910xx_set_initsoc(pkp6910xx, value);
}

int32_t kp6910xx_set_initcycle(uint16_t value)
{
    return _kp6910xx_set_cyclecnt(pkp6910xx, value);
}

int32_t kp6910xx_set_extcharger(int16_t value)
{
    //pkp6910xx->pi2c_intf->debug_func("set charger:%d\r\n",value);
    return _kp6910xx_set_extchargersts(pkp6910xx, value);
}
static uint32_t  kp6910xx_crc32(uint32_t data, uint32_t crcin)
{
#define	CRC32_POLY				0x04C11DB7

    uint32_t crc32 = data ^ crcin;
    uint32_t	index;

    for (index = 0; index < 32; index++)				/* Prepare to rotate 32 bits */
    {
        if (crc32 & 0x80000000)										/* b31 is set... */
            crc32 = (crc32 << 1) ^ CRC32_POLY;			/* rotate and XOR with polynomic */
        else                          						/* b31 is clear... */
            crc32 = (crc32 << 1);										/* just rotate */
    }																						/* Loop for 32 bits */
    //	crc &= 0xFFFF;													/* Ensure CRC remains 32-bit value */

    return (crc32);															/* Return updated CRC */

}

static uint32_t kp6910xx_crc32_modbus(uint8_t *puchMsg, uint32_t usDataLen)
{
    uint32_t wCRCin = 0xFFFFFFFF;  //0x0000; //
    uint32_t uIndx = 0;
    uint32_t *pBuff = (uint32_t *)puchMsg;

    for (uIndx = 0; uIndx < usDataLen / 4; uIndx++)
    {
        wCRCin = kp6910xx_crc32(*pBuff, wCRCin);
        pBuff++;
    }

    return (wCRCin);
}

//--------------------------------------------------fw upgrade driver task code start--------------------------------------------------//
#if FW_UPGRADE
static int32_t  kp6910xx_bl_calc_checksum(KP6910XX_T *chip)
{
    if(chip->pfwbaseaddr == NULL)
        return 1;

    chip->checksum = kp6910xx_crc32_modbus(chip->pfwbaseaddr->code_area, CODE_SIZE + 4);

    chip->pi2c_intf->debug_func("Fw CheckSum:%x\r\n", chip->checksum);

    return 0;
}

static int32_t kp6910xx_bl_update_firmware(KP6910XX_T *chip)
{
    uint32_t  uPackNum = 0;
    uint32_t  uTotal_Size = sizeof(FW_STRUCT_T);
    uint32_t  uIndx = 0;
    uint8_t   uDataBuff[36];
    uint32_t  uRWAddr = 0;
    uint8_t 	uRet = 0;
    uint8_t   *pDataBuf = (uint8_t *)chip->pfwbaseaddr;
    uint8_t 	uBuffer[2] = {0};

#define PACKSIZE	   32
#define BASEADDR     0x1000

    if(kp6910xx_bl_calc_checksum(chip))
        return 1;

    uPackNum = uTotal_Size / PACKSIZE;

    for(uIndx = 0; uIndx < uPackNum; uIndx++)
    {

        uRWAddr = BASEADDR + uIndx * PACKSIZE;
        uRWAddr = uRWAddr & 0xffff;

        uDataBuff[0] = uRWAddr & 0xff;
        uDataBuff[1] = (uRWAddr >> 8) & 0xff;
        uDataBuff[2] = PACKSIZE & 0xff;
        uDataBuff[3] = (PACKSIZE >> 8) & 0xff;

        if(uIndx == (CODE_SIZE / 32))
        {
            memcpy(uDataBuff + 4, pDataBuf + uIndx * PACKSIZE, PACKSIZE - 4);
            memcpy(uDataBuff + 4 + PACKSIZE - 4, (uint8_t *)&chip->checksum, 4);
        }
        else if(uIndx == (uPackNum - 1))
        {
            memcpy(uDataBuff + 4, pDataBuf + uIndx * PACKSIZE, PACKSIZE - CALI_SIZE);
            memcpy(uDataBuff + 4 + CALI_SIZE, (uint8_t *)&chip->cali_data, CALI_SIZE);
        }
        else
            memcpy(uDataBuff + 4, pDataBuf + uIndx * PACKSIZE, PACKSIZE);


        if ((uRet = chip->pi2c_intf->i2c_write_func(SBS04_UPDATEDATA, uDataBuff, 36)) != 0)
        {
            chip->pi2c_intf->debug_func("Fw Packet %d/%d Send Failed\r\n", uIndx, uPackNum);
            break;
        }

        chip->pi2c_intf->delay_func(2);


        if ((uRet = chip->pi2c_intf->i2c_read_func(SBS00_MESSAGEACK, uBuffer, 2)) != 0)
        {
            chip->pi2c_intf->debug_func("Fw Packet %d/%d ACK Failed\r\n", uIndx, uPackNum);
            break;
        }

        chip->pi2c_intf->delay_func(2);

        //		chip->pi2c_intf->debug_func("Fw packet %d/%d Send Successfully\r\n",uIndx,uPackNum);
    }

    return uRet;
}

static int32_t kp6910xx_bl_enter_downloadmode(KP6910XX_T *chip)
{
    uint8_t 	buffer[2] ;
    uint32_t 	retry = 20;
    uint8_t 	ret = 0;
    uint8_t   indx = 0;
    uint16_t  retsts = 0;

    buffer[indx++] = 0x26;
    buffer[indx++] = 0x00;

    ret =  chip->pi2c_intf->i2c_write_func(0x44, buffer, 2);
    chip->pi2c_intf->delay_func(30);
    //} while (ret != ERR_SUCCESSFUL && (--retry));

    if(ret != 0)
    {
        chip->pi2c_intf->debug_func("Software Failed\r\n");
        return ret;
    }

    chip->pi2c_intf->delay_func(210); // 210 ms delay

    indx = 0;
    buffer[indx++] = 0x01;
    buffer[indx++] = 0x00;

    if ((ret = chip->pi2c_intf->i2c_write_func(SBS01_ENTERDOWNLOAD, buffer, 2)) != 0)
    {
        chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return ret;
    }

    chip->pi2c_intf->delay_func(2);

    if ((ret = chip->pi2c_intf->i2c_read_func(SBS00_MESSAGEACK, buffer, 2)) != 0)
    {
        chip->pi2c_intf->debug_func("ACK Failed \r\n");
        return ret;
    }

    retsts = buffer[1] << 8 | buffer[0];

    if (retsts)
    {
        chip->pi2c_intf->debug_func("Enter Download Mode Failed\r\n");
        return retsts;
    }

    return 0;
}

static int32_t kp6910xx_bl_get_calidata(KP6910XX_T *chip)
{
    uint8_t 	buffer[4];
    uint8_t 	ret = 0;
    uint8_t   indx = 0;

    buffer[indx++] = CALI_ADDRESS & 0xFF;
    buffer[indx++] = (CALI_ADDRESS >> 8) & 0xff ;
    buffer[indx++] = (CALI_ADDRESS >> 16) & 0xff ;
    buffer[indx++] = (CALI_ADDRESS >> 24) & 0xff ;

    if ((ret = chip->pi2c_intf->i2c_write_func(SBS02_SETREADADDR, buffer, 4)) != 0)
    {
        chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return ret;
    }

    chip->pi2c_intf->delay_func(1);

    if ((ret = chip->pi2c_intf->i2c_read_func(SBS03_READDATACMD, chip->cali_data, CALI_SIZE - 2)) != 0)
    {
        chip->pi2c_intf->debug_func("Read Data Failed \r\n");
        return ret;
    }



    return 0;

}

static int32_t kp6910xx_bl_get_checkresult(KP6910XX_T *chip)
{
    uint8_t 	buffer[2];
    uint8_t   indx = 0;
    uint16_t  retsts = 0;
    uint8_t 	ret = 0;


    buffer[indx++] = 0x01;
    buffer[indx++] = 0x00;

    if ((ret = chip->pi2c_intf->i2c_write_func(SBS05_CHECKCRC, buffer, 2)) != 0)
    {
        chip->pi2c_intf->debug_func("Send Command Failed \r\n");
        return ret;
    }

    chip->pi2c_intf->delay_func(5);


    if ((ret = chip->pi2c_intf->i2c_read_func(SBS00_MESSAGEACK, buffer, 2)) != 0)
    {
        chip->pi2c_intf->debug_func("ACK Failed \r\n");
        return ret;
    }

    retsts = buffer[1] << 8 | buffer[0];

    if (retsts)
    {
        chip->pi2c_intf->debug_func("Crc16 Failed\r\n");
        return retsts;
    }

    return 0;
}

static int32_t kp6910xx_bl_jump_toapp(KP6910XX_T *chip)
{
    uint8_t 	buffer[2];
    uint8_t   indx = 0;
    uint8_t 	ret = 0;

    buffer[indx++] = 0x01;
    buffer[indx++] = 0x00;

    if ((ret = chip->pi2c_intf->i2c_write_func(SBS06_FWRUNING, buffer, 2)) != 0)
    {
        chip->pi2c_intf->debug_func("Send Command Failed\r\n");
        return ret;
    }

    return 0;
}


static int32_t  kp6910xx_bl_check_fwversion(KP6910XX_T *chip)
{
    int32_t  ret = 0;

    if(chip->pfwbaseaddr == NULL)
        return 1;

    ret = _kp6910xx_get_fwversion(chip);
    if(ret != 0)
        return 1;

    if((chip->pfwbaseaddr->fwversion > chip->fwversion))
        chip->update_flag = 1;


    if((chip->fwversion & 0xFFFF) == 0x4C42)
        chip->bl_stage = 1;

    if (chip->bl_stage == 1)
    {
        chip->bl_stage = 0;
        chip->pi2c_intf->delay_func(250);
        _kp6910xx_get_fwversion(chip);

        if((chip->fwversion & 0xFFFF) == 0x4C42)
            chip->bl_stage = 1;

    }

    chip->pi2c_intf->debug_func("update_flag: %d, bl_stage : %x\r\n", chip->update_flag, chip->bl_stage);


    return 0;

}


int32_t kp6910xx_upgrade_task(void)
{
    int32_t  ret = 0;

    //
    if(kp6910xx_bl_check_fwversion(pkp6910xx) )
        return 1;

    if(pkp6910xx->update_flag || pkp6910xx->bl_stage )
    {

        if(!pkp6910xx->bl_stage)
        {
            ret = kp6910xx_bl_enter_downloadmode(pkp6910xx);
            if(ret)
                goto err;
        }

        ret = kp6910xx_bl_get_calidata(pkp6910xx);
        if(ret)
            goto err;
        pkp6910xx->pi2c_intf->debug_func("update_fw\r\n");
        ret = kp6910xx_bl_update_firmware(pkp6910xx);
        if(ret)
            goto err;

        pkp6910xx->pi2c_intf->debug_func("check_fw\r\n");
        ret = kp6910xx_bl_get_checkresult(pkp6910xx);
        if(ret)
            goto err;

        pkp6910xx->pi2c_intf->debug_func("jumptoapp\r\n");
        ret = kp6910xx_bl_jump_toapp(pkp6910xx);
        if(ret)
            goto err;



        pkp6910xx->pi2c_intf->debug_func("Firmware Upgrade Successfully\r\n");
        pkp6910xx->pi2c_intf->delay_func(200);
        return 0;

err:
        pkp6910xx->pi2c_intf->debug_func("Firmware Upgrade Failed\r\n");

        return 1;
    }

    return 0;
}
#endif
//--------------------------------------------------fw upgrade driver task code end--------------------------------------------------//
int32_t kp6910xx_modify_batterymodel(uint8_t*  batt_table_model)
{
	uint8_t  uDataBuff[DATA_SIZE];
    uint8_t  uRet = 0;
    uint32_t uTable1_Crc32 = 0, uTable2_Crc32 = 0;

	//step1 check tabledata is valid
	uTable1_Crc32 = batt_table_model[0] | batt_table_model[1] << 8 | batt_table_model[2] << 16 | batt_table_model[3] << 24;
	uTable2_Crc32 = kp6910xx_crc32_modbus(batt_table_model+4, DATA_SIZE-4);
	pkp6910xx->pi2c_intf->debug_func("real CRC 0x%08x, calc CRC 0x%08x\r\n", uTable1_Crc32, uTable2_Crc32);
	if(uTable1_Crc32 != uTable2_Crc32)
	{
		return 1;
	}
	
	//step2 write table model
	kp6910xx_set_tabledata(pkp6910xx, TABLE_START_ADDR, batt_table_model, DATA_SIZE);
	
	//step3 read and check table model
	uRet = kp6910xx_get_tabledata(pkp6910xx, TABLE_START_ADDR, uDataBuff, DATA_SIZE);
	
	uTable2_Crc32 = kp6910xx_crc32_modbus(uDataBuff+4, DATA_SIZE-4);
	
	pkp6910xx->pi2c_intf->debug_func("Table write CRC 0x%08x, read CRC 0x%08x\r\n", uTable1_Crc32, uTable2_Crc32);
    if((uTable1_Crc32 == uTable2_Crc32)  && (!uRet))
	{
        return 0;
    }
    else
    {
        return 1;
    }
}

int32_t kp6910xx_modify_batterparams(uint8_t*  batt_cfg_params)
{
	uint8_t  uDataBuff[PARAMS_SIZE];
	uint8_t  uRet = 0;
	uint32_t uParams1_Crc32 = 0, uParams2_Crc32 = 0;
	
	//step1 check data CRC data
	uParams1_Crc32 = kp6910xx_crc32_modbus(batt_cfg_params, PARAMS_SIZE);
	
	//step2 write data 
	kp6910xx_set_tabledata(pkp6910xx, PARAMS_START_ADDR, batt_cfg_params, PARAMS_SIZE);
	
	//step3 read data and check 
	uRet = kp6910xx_get_tabledata(pkp6910xx, PARAMS_START_ADDR, uDataBuff, PARAMS_SIZE);
	
	uParams2_Crc32 = kp6910xx_crc32_modbus(uDataBuff, PARAMS_SIZE);
	
	pkp6910xx->pi2c_intf->debug_func("Params write CRC 0x%08x, read CRC 0x%08x\r\n", uParams1_Crc32, uParams2_Crc32);
    if((uParams1_Crc32 == uParams2_Crc32)  && (!uRet))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


int32_t kp6910xx_modify_cfgparams(uint16_t cfg_addr , uint16_t cfg_data)
{
    uint8_t  uRet = 0;
    uint16_t uData = 0;

    uRet = kp6910xx_get_paramdata(pkp6910xx, cfg_addr, &uData);
    pkp6910xx->pi2c_intf->debug_func("get cfg %04x \r\n", uData);
    if (uData != cfg_data)
    {
        kp6910xx_set_paramdata(pkp6910xx, cfg_addr, cfg_data);
        uRet = kp6910xx_get_paramdata(pkp6910xx, cfg_addr, &uData);
        pkp6910xx->pi2c_intf->debug_func("get cfg %04x \r\n", uData);
    }

    if((uData == cfg_data)  && (!uRet))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int32_t  kp6910xx_modifydata_task(void)
{   
	uint16_t uData = 0;
	uint16_t cfg_addr, cfg_data;
    int32_t ret = 0;
    int32_t retry = 3;
	
	//1. check params already set before
	kp6910xx_get_paramdata(pkp6910xx, MF_DATE_ADDR, &uData);
	pkp6910xx->pi2c_intf->debug_func("check MF_DATE 0x%04x, 0x%04x \r\n",uData, batt_mf_date);
	if(uData == batt_mf_date)
	{
		return 0;
	}
	
	//2. set prj params
    do
    {
		ret = 0;
		
		ret |= kp6910xx_modify_batterymodel(batt_table_model);

		ret |= kp6910xx_modify_batterparams(batt_cfg_params);
		//[todo]���ʵ����������ã����²�ͬ�ļĴ�����ַ,���¶�Ӧ��ֵ���ɡ������ַ�ɲο�Ӧ��ָ���ֲ�
		//eg.�޸ķŵ��ֹ��ѹ3200
		cfg_addr = 0x5F1A;
		cfg_data = 3200;
		ret |= kp6910xx_modify_cfgparams(cfg_addr, cfg_data);
		

        if (!ret)
        {
			kp6910xx_set_paramdata(pkp6910xx, MF_DATE_ADDR, batt_mf_date);
			
			pkp6910xx->pi2c_intf->debug_func("CFG Task Pass\r\n",batt_mf_date);
			
			//�������֮�󣬸�λʹ������Ч
			kp6910xx_set_resetcmd();
            return  retry ? 0 : 1;
        }
    }
    while(--retry);
	
    return  retry ? 0 : 1;
}
