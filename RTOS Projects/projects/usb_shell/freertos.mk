
RTOS_ROOT = $(SDK_ROOT)/external/freertos

RTOS_SRC_DIRS := $(RTOS_ROOT)/source \
				$(RTOS_ROOT)/portable/CMSIS/nrf52 \
				$(RTOS_ROOT)/portable/GCC/nrf52/


RTOS_SRC := $(foreach folder, $(RTOS_SRC_DIRS), $(notdir $(wildcard $(folder)/*.c)))


RTOS_SRC_DIRS += $(RTOS_ROOT)/source/portable/MemMang
RTOS_SRC += heap_4.c

RTOS_INC_DIRS := $(RTOS_ROOT)/source/include
RTOS_INC_DIRS += $(RTOS_ROOT)/source/portable/MemMang
RTOS_INC_DIRS += $(RTOS_ROOT)/portable/CMSIS/nrf52
RTOS_INC_DIRS += $(RTOS_ROOT)/portable/GCC/nrf52

CC_SRC += $(RTOS_SRC)
SRC_DIRS += $(RTOS_SRC_DIRS)
INC_FOLDERS += $(RTOS_INC_DIRS)
