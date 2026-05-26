#pragma once

#include_next <mcuconf.h>

#undef AT32_ADC_USE_ADC1
#define AT32_ADC_USE_ADC1 TRUE

#undef AT32_SPI_USE_SPI1
#define AT32_SPI_USE_SPI1 TRUE

#undef AT32_I2C_USE_I2C1
#define AT32_I2C_USE_I2C1 TRUE

