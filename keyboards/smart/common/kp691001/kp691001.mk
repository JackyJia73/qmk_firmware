I2C_DRIVER_REQUIRED = yes
OPT_DEFS += -DKP691001_ENABLE

KP691001_DIR = common/kp691001
SRC += \
     $(KP691001_DIR)/kp691001driver.c \
     $(KP691001_DIR)/kp6910xx.c \
     $(KP691001_DIR)/batt_model.c 

VPATH += $(TOP_DIR)/keyboards/smart/$(KP691001_DIR)

