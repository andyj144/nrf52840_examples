PROJECT_NAME     := usb_shell
OUTPUT_DIRECTORY := output
BUILD_DIRECTORY := _build

GNU_INSTALL_ROOT ?= $(GCC_ARM)/bin/
GNU_PREFIX ?= arm-none-eabi
REPO_ROOT := ../../

SDK_ROOT := $(NRF_SDK)

TASK_DIR := $(REPO_ROOT)/tasks
PRINTF := $(REPO_ROOT)/printf
PROJ_DIR := .

LINKER_SCRIPT  := generic_gcc_nrf52.ld

SRC_DIRS := $(PROJ_DIR)  $(TASK_DIR)

SRC_DIRS += $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
	$(SDK_ROOT)/components/libraries/atomic_flags \
	$(SDK_ROOT)/components/libraries/util \
	$(SDK_ROOT)/components/libraries/hardfault/nrf52/handler \
  $(SDK_ROOT)/components/libraries/hardfault \

SRC_DIRS += $(SDK_ROOT)/components/libraries/usbd \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \

SRC_DIRS += $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx/drivers/src \
  $(SDK_ROOT)/modules/nrfx/soc \
	$(SDK_ROOT)/modules/nrfx/mdk/


SRC_DIRS += $(PRINTF)

CC_SRC := main.c \
  usb_task.c \
  shell_task.c \
  blinky_task.c \
  nrf_atfifo.c \
	nrf_atflags.c\
	nrf_atomic.c \
	nrfx_atomic.c \
  nrfx_clock.c \
  nrfx_power.c \
  nrfx_usbd.c \
  nrf_drv_power.c \
  nrf_drv_clock.c \
  app_usbd.c \
  app_usbd_core.c \
  app_usbd_serial_num.c \
  app_usbd_string_desc.c \
  app_usbd_cdc_acm.c \
	app_util_platform.c \
	app_error.c \
	app_error_weak.c \
	app_error_handler_gcc.c \
	hardfault_handler_gcc.c \
	hardfault_implementation.c \
  system_nrf52840.c \
  printf.c


AS_SRC += gcc_startup_nrf52840.S


INC_FOLDERS += \
	$(PROJ_DIR) \
	$(PROJ_DIR)/config \
	$(SDK_ROOT)/modules/nrfx \
	$(SDK_ROOT)/modules/nrfx/hal \
	$(SDK_ROOT)/modules/nrfx/drivers/include \
	$(SDK_ROOT)/modules/nrfx/mdk \
	$(SDK_ROOT)/integration/nrfx \
	$(SDK_ROOT)/components/toolchain/cmsis/include \
	$(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
	$(SDK_ROOT)/external/utf_converter \
  ${PRINTF}

INC_FOLDERS += $(shell find $(SDK_ROOT)/components/libraries -type d)

INC_FOLDERS += $(SRC_DIRS)

include freertos.mk

LIB_FILES += \

OPT = -g3

# OPT = -O3
# Uncomment the line below to enable link time optimization
# OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBSP_DEFINES_ONLY
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52840_XXAA
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
# CFLAGS += -Wall -Werror
CFLAGS += -Wall
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBSP_DEFINES_ONLY
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52840_XXAA

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs


CC      := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-gcc
CXX     := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-c++
AS      := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-as
AR      := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ar -r
LD      := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ld
NM      := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-nm
OBJDUMP := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objdump
OBJCOPY := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objcopy
SIZE    := $(GNU_INSTALL_ROOT)$(GNU_PREFIX)-size

CFLAGS += -D__HEAP_SIZE=8192
CFLAGS += -D__STACK_SIZE=8192
ASMFLAGS += -D__HEAP_SIZE=8192
ASMFLAGS += -D__STACK_SIZE=8192

LIB_FILES += -lc -lnosys -lm

OBJS = $(CC_SRC:.c=.o)
OBJS += $(AS_SRC:.S=.o)

VPATH = $(SRC_DIRS) $(RTOS_SRC)
default: $(OUTPUT_DIRECTORY) $(BUILD_DIRECTORY) $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex

$(OUTPUT_DIRECTORY):
	mkdir $@

$(BUILD_DIRECTORY):
	mkdir $@
	
$(OUTPUT_DIRECTORY)/%.hex: $(OUTPUT_DIRECTORY)/%.out
	@$(OBJCOPY) -O ihex $< $@

$(OUTPUT_DIRECTORY)/%.out:$(addprefix $(BUILD_DIRECTORY)/, $(OBJS))
	@echo Linking $@
	@$(CC) $(LDFLAGS) $^ $(LIB_FILES) -Wl,-Map=$(@:.out=.map) -o $@

$(BUILD_DIRECTORY)/%.o:%.c
	@echo building $<
	@$(CC) -std=c99 -MP -MD -c -o $@ $^ $(CFLAGS) $(addprefix -I, $(INC_FOLDERS))

$(BUILD_DIRECTORY)/%.o:%.S
	@echo building $<
	@$(CC) -x assembler-with-cpp -MP -MD -c -o $@ $^ $(ASMFLAGS) $(addprefix -I, $(INC_FOLDERS))

SDK_CONFIG_FILE := ./config/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)

.PHONY:dfu-package

_dfu: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	@echo Packaging $<
	adafruit-nrfutil dfu genpkg --sd-req 0xFFFE --dev-type 0x0052 --application $<  $(OUTPUT_DIRECTORY)/dfu-package.zip

dfu-package: $(OUTPUT_DIRECTORY) _dfu

bootload: default dfu-package
	@echo Flashing: $<
	adafruit-nrfutil --verbose dfu serial --package $(OUTPUT_DIRECTORY)/dfu-package.zip -p $(SERIAL_PORT) -b 115200 --singlebank --touch 1200

.PHONY: clean

.PRECIOUS: $(addprefix $(BUILD_DIRECTORY)/, $(OBJS)) $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex $(OUTPUT_DIRECTORY)/nrf52840_xxaa.out

clean:
	rm -rf $(BUILD_DIRECTORY)
	rm -rf $(OUTPUT_DIRECTORY)