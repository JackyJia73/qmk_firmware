#ifndef __KP6910XX_H_
#define __KP6910XX_H_

#include <stdint.h>


#define  LSB_VOLT          305
#define  VOLT_FACTOR       1000

#define  CODE_SIZE          		0x4DF8
#define  DATA_SIZE          		0x100
#define  PARAMS_SIZE          		0x70
#define  CALI_ADDRESS 				0x08005F70
#define  TABLE_START_ADDR       	0x5E00
#define  TABLE_END_ADDR         	0x5F00
#define  PARAMS_START_ADDR      	0x5F00
#define  MF_DATE_ADDR           	0x5F20



#define   MF_OFFSET            (MF_DATE_ADDR-TABLE_START_ADDR)
#define CALI_SIZE              0x10


typedef struct
{
    int32_t (*i2c_write_func)(uint8_t reg, uint8_t *sendbuf, uint32_t lens);
    int32_t (*i2c_read_func)(uint8_t reg, uint8_t *recvbuf, uint32_t lens);
    void (*delay_func)(uint32_t time); // uint ms
    int32_t (*debug_func)(const char *format, ...);
} I2C_READ_WRITE_T;


typedef struct
{
    uint8_t  code_area[CODE_SIZE];
    uint32_t fwversion;
    uint32_t check_sum;
    uint8_t  data_area[DATA_SIZE];
} FW_STRUCT_T;


typedef struct
{
    I2C_READ_WRITE_T 		*pi2c_intf;
    float  					rsoc;
    uint16_t  				cell_volt;
    uint16_t  				atte;
    int16_t  				inttemp;
    uint16_t  				icversion;
    uint16_t                alert_flag;
    uint32_t  				fwversion;
    FW_STRUCT_T 			*pfwbaseaddr;
    uint32_t  				checksum;
    uint32_t  				update_flag;
    uint32_t  				bl_stage;
    uint32_t          		table_crc;
    uint16_t          		param_date;
    uint8_t           		cali_data[CALI_SIZE];

} KP6910XX_T;

#define   	SBS00_ICVERSION  				0x00
#define	  	SBS02_VCELL  			 	    0x02
#define   	SBS04_SOC  						0x04
#define	  	SBS06_ALERT			  	        0x06
#define	  	SBS08_CONFIG			  	    0x08
#define	  	SBS0A_MODE			  	        0x0A

#define   	SBS44_MFGBLKACCESS  			0x44
#define	  	SBS81_EXTCHRGER 			  	0x81
#define	  	SBS82_EXTINITSOC 			  	0x82
#define	  	SBS83_EXTCYCLECNT			  	0x83
#define	  	SBSF0_FWVER			  	        0xF0

#define     SBS00_MESSAGEACK      			0x00				//37+4 
#define		SBS01_ENTERDOWNLOAD			  	0x01          //0, I
#define		SBS02_SETREADADDR       		0x02  			//1, V
#define		SBS03_READDATACMD       		0x03 				//2, V
#define		SBS04_UPDATEDATA        		0x04				//3, V
#define		SBS05_CHECKCRC        			0x05 				//3, V	
#define		SBS06_FWRUNING       		  	0x06  			//3, V


#define   SBS44_MAX_VALID_DATA_SIZE  0x20




int32_t 	kp6910xx_get_dump_msg(void);

int32_t 	kp6910xx_get_voltage(void);

int32_t 	kp6910xx_get_temperature(void);

int32_t  	kp6910xx_get_rsoc(void);

int32_t 	kp6910xx_clear_alert(void);

int32_t 	kp6910xx_set_alertthresh(uint8_t value);

int32_t 	kp6910xx_set_wakeupcmd(void);

int32_t 	kp6910xx_set_sleepcmd(void);

int32_t 	kp6910xx_set_qstart(void);

int32_t 	kp6910xx_set_resetcmd(void);

int32_t 	kp6910xx_set_initsoc(int16_t value);

int32_t 	kp6910xx_set_initcycle(uint16_t value);

int32_t 	kp6910xx_set_extcharger(int16_t value);

int32_t 	kp6910xx_intf_register(KP6910XX_T *kp6910xx, I2C_READ_WRITE_T *pi2c_intf, FW_STRUCT_T *pfw);

int32_t 	kp6910xx_upgrade_task(void);

int32_t 	kp6910xx_get_tableinfo(void);

int32_t  	kp6910xx_modifydata_task(void);

int32_t 	kp6910xx_set_tabledata(KP6910XX_T *chip, uint16_t addr, uint8_t *pdata, uint16_t len );

int32_t 	kp6910xx_get_tabledata(KP6910XX_T *chip, uint16_t addr, uint8_t *pdata, uint16_t len );

#endif
