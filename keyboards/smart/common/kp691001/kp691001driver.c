#include "quantum.h"
#include "i2c_master.h"
#include "battery_driver.h"
#include "kp6910xx.h"



#define  I2C_ADDRESS           0xC4

KP6910XX_T kp6910xx;

extern const unsigned char fw_bin[];			// 如果需要通过主机升级电量计固件，需要指定固件地址

// 需要加重发机制 保证通信稳定性
int32_t i2c_write(uint8_t reg,uint8_t *sendbuf,uint32_t lens)
{
	uint8_t retry = 3;
	int32_t ret = 0;
	
	while(retry -- )
	{
		if(I2C_STATUS_SUCCESS == i2c_write_register(I2C_ADDRESS,reg,sendbuf,lens,10))
			break;
		wait_ms(1);
  }
	
	if(retry <= 0)
	  ret = -1;
	
	return ret;
}


// 需要加重发机制 保证通信稳定性
int32_t i2c_read(uint8_t reg, uint8_t *recvbuf,uint32_t lens)
{
	uint8_t retry = 3;
	int32_t ret = 0;

  while(retry -- )	
	{
		if(I2C_STATUS_SUCCESS == i2c_read_register(I2C_ADDRESS,reg,recvbuf,lens,10))
			break;
		wait_ms(1);
  }
	
	if(retry <= 0)
	  ret = -1;
	
	return ret;

}

void my_wait_ms(uint32_t ms)
{
	wait_ms(ms);
};

int32_t my_uprintf(const char *format, ...)
{
	char buffer[128]; // 定义缓存区
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args); // 格式化字符串到buffer
    va_end(args);
	uprintf(buffer);
	uprintf("\n");
	return 0;
};

static I2C_READ_WRITE_T bird_intf = {
	
	.i2c_write_func = i2c_write,
	.i2c_read_func  = i2c_read,
	.delay_func     = my_wait_ms,
	.debug_func     = my_uprintf,
	
};

void battery_driver_init(void) {

		// uprintf("Init I2C");
		i2c_init();
		// uprintf("Init kp691001 \r\n");
		if( 0 != kp6910xx_intf_register(&kp6910xx,&bird_intf,NULL)) // 如果不需要升级逻辑，第三个参数填 NULL 
		 	uprintf(" Register i2c func failed\r\n");
	    //uprintf("Start update battery model data ...\r\n");
		uprintf((0!=kp6910xx_modifydata_task())?"Fail!!\r\n":"Success.\r\n");	//更新电池参数模型, 更新配置参数(示例:修改容量)
	
		//kp6910xx_upgrade_task(); // 不需要升级逻辑可以去掉
		//kp6910xx_set_initsoc(5000);
		//kp6910xx_set_initcycle(10);
		//if (gpio_read_pin(PLUG_IN)==0)
        kp6910xx_set_extcharger(-1);	//设置充电状态； 1充电器插入，3充电器满充，-1充电器拔出
		//kp6910xx_set_alertthresh(20); // 根据需求设置报警阈值
		
		//kp6910xx_get_dump_msg();
		//rsoc = kp6910xx_get_rsoc();
		//kp691001_inited=true;  
	
}

uint8_t battery_driver_sample_percent(void) {
#ifdef KP691001_FULL_PIN
	if(!gpio_read_pin(FULL)){
			kp6910xx_set_extcharger(3); //设置充电状态； 1充电器插入，3充电器满充，-1充电器拔出
		} 
#endif

#ifdef KP691001_CHARGING_PIN	
		{
			kp6910xx_set_extcharger(gpio_read_pin(PLUG_IN)?1:-1);	//设置充电状态； 1充电器插入，3充电器满充，-1充电器拔出		
			uprintf("charger %ld\r\n",gpio_read_pin(PLUG_IN));
		}
#endif


	kp6910xx_get_dump_msg();
	return kp6910xx_get_rsoc();
}


