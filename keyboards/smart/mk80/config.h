#pragma once
//#define DEBUG_MATRIX_SCAN_RATE

#define I2C_DRIVER	        I2CD1
#define I2C1_SCL_PIN        B6
#define I2C1_SCL_PAL_MODE	4
#define I2C1_SDA_PIN        B7
#define I2C1_SDA_PAL_MODE	4
#define I2C1_TIMINGR_PRESC  5U
#define I2C1_TIMINGR_SCLDEL 3U
#define I2C1_TIMINGR_SDADEL 0U
#define I2C1_TIMINGR_SCLH   10U
#define I2C1_TIMINGR_SCLL   32U


#if defined(RGB_MATRIX_ENABLE)
/* SPI Config for LED Driver */
    #define SPI_DRIVER SPID1
    #define SPI_SCK_PIN A5
    #define SPI_MISO_PIN A6
    #define SPI_MOSI_PIN A7

    #define SPI_SCK_PAL_MODE 5
    #define SPI_MOSI_PAL_MODE	5
    #define SPI_MISO_PAL_MODE	5
    #define AW20216S_SPI_MODE	0
    #define AW20216S_SPI_DIVISOR	32

    #define AW20216S_CS_PIN_1 A11
    #define AW20216S_CS_PIN_2 D2
    #define AW20216S_EN_PIN B3

#    define RGB_MATRIX_KEYPRESSES
#    define RGB_MATRIX_FRAMEBUFFER_EFFECTS

#endif


#ifdef LK_WIRELESS_ENABLE
/* Hardware configuration */
#    define P24G_MODE_SELECT_PIN C12
#    define BT_MODE_SELECT_PIN C13

//#    define LKBT51_RESET_PIN C6
#    define LKBT51_INT_INPUT_PIN B5
#    define LKBT51_INT_OUTPUT_PIN C4

#    define USB_POWER_SENSE_PIN B9
#    define USB_POWER_CONNECTED_LEVEL 1


/* Keep USB connection in wireless mode */
#    define KEEP_USB_CONNECTION_IN_WIRELESS_MODE

#    define DISABLE_REPORT_BUFFER

#    define RETPORT_RETRY_COUNT 0
#endif

/* Factory test keys */

#define MATRIX_IO_DELAY 10
