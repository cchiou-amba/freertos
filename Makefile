##
## Copyright (c) 2023 Ambarella, Inc.
##
## This file and its contents ("Software") are protected by intellectual
## property rights including, without limitation, U.S. and/or foreign
## copyrights. This Software is also the confidential and proprietary
## information of Ambarella, Inc. and its licensors. You may not use, reproduce,
## disclose, distribute, modify, or otherwise prepare derivative works of this
## Software or any portion thereof except pursuant to a signed license agreement
## or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
## In the absence of such an agreement, you agree to promptly notify and return
## this Software to Ambarella, Inc.
##
## THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
## INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
## MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
## IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
## INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##

# Build configuration
.PHONY: all clean install

sinclude $(DOT_CONFIG)

# Output configuration
TARGET_ELF	:= $(MCU_OUTPUT)/$(ARCH).elf
TARGET_BIN	:= $(MCU_OUTPUT)/$(ARCH).bin
TARGET_HDR_BIN	:= $(MCU_OUTPUT)/$(ARCH)_HDR.bin
CHIP_UPPER_CASE	:= $(shell grep ^CONFIG_CHIP $(DOT_CONFIG) | sed -e s/^CONFIG_CHIP_// | sed -e s/=y//)
CHIP_LOWER_CASE	:= $(shell echo $(CHIP_UPPER_CASE) | tr [:upper:] [:lower:])
ARCH_LOWER_CASE	:= $(shell echo $(ARCH) | tr [:upper:] [:lower:])

# Toolchain configuration
ifeq ($(MCU_CROSS_COMPILE),)
CROSS_TOOL	:= /usr/local/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-
else
CROSS_TOOL	:= $(MCU_CROSS_COMPILE)
endif

# Compiler tools
MCU_CC		:= $(CROSS_TOOL)gcc
MCU_CPP		:= $(CROSS_TOOL)gcc -E
MCU_NM		:= $(CROSS_TOOL)nm
MCU_SZ		:= $(CROSS_TOOL)size
MCU_STRIP	:= $(CROSS_TOOL)strip
MCU_OBJCOPY	:= $(CROSS_TOOL)objcopy
MCU_OBJDUMP	:= $(CROSS_TOOL)objdump

# Common compiler flags
MCU_CMN_FLAGS	:= \
	-DARCH_NAME=\"$(ARCH)\" \
	-fno-common \
	-ffunction-sections -fdata-sections \
	-fomit-frame-pointer \
	-fno-builtin -ffreestanding \
	-fno-exceptions -fno-unwind-tables \
	-fno-asynchronous-unwind-tables \
	-DCONFIG_CHIP_$(CHIP_UPPER_CASE) \
	-DCONFIG_BOOT_$(ARCH) \
	-DRTOS_BOOT_ADDR=$(BOOT_ADDR)

# Architecture-specific flags
ifeq ($(ARCH),CM3)
MCU_CMN_FLAGS	+= \
	-mcpu=cortex-m3 -mthumb -DARCH_CM3 \
	--specs=nano.specs --specs=nosys.specs
else ifeq ($(ARCH),R52)
MCU_CMN_FLAGS	+= \
	-mcpu=cortex-r52 -marm -mfloat-abi=soft -DARCH_R52
endif

# Compiler and linker flags
MCU_CFLAGS	:= $(MCU_CMN_FLAGS) -Wall -Os
MCU_AFLAGS	:= $(MCU_CMN_FLAGS) -D__ASM__
MCU_LDFLAGS	:= \
	$(MCU_CMN_FLAGS) \
	-nostartfiles -nostdlib -lgcc -lm -lc \
	-Wl,--gc-sections -Wl,--build-id=none -Bstatic

# Linker script selection
MCU_LINKER := $(MCU_OUTPUT)/linker.ld
ifneq ($(wildcard plat/$(ARCH)/$(CHIP_LOWER_CASE)/$(CHIP_LOWER_CASE).ld.cpp),)
MCU_LINKER_SRC := plat/$(ARCH)/$(CHIP_LOWER_CASE)/$(CHIP_LOWER_CASE).ld.cpp
else
ifneq ($(wildcard plat/$(ARCH)/cmn/$(ARCH_LOWER_CASE)_def.ld.cpp),)
MCU_LINKER_SRC := plat/$(ARCH)/cmn/$(ARCH_LOWER_CASE)_def.ld.cpp
else
$(error "No linker script defined")
endif
endif

MCU_LDFLAGS	+= -T$(MCU_LINKER)

# FreeRTOS source files
FREERTOS_FILES	:= \
	FreeRTOS/tasks.c \
	FreeRTOS/list.c \
	FreeRTOS/queue.c \
	FreeRTOS/timers.c \
	FreeRTOS/event_groups.c \
	FreeRTOS/croutine.c \
	FreeRTOS/stream_buffer.c \
	FreeRTOS/portable/GCC/ARM_$(ARCH)/port.c

# Add architecture-specific ASM files if they exist
ifneq ($(wildcard FreeRTOS/portable/GCC/ARM_$(ARCH)/portASM.S),)
FREERTOS_FILES	+= FreeRTOS/portable/GCC/ARM_$(ARCH)/portASM.S
endif

# Platform and driver source files
MCU_FILES	:= $(FREERTOS_FILES)
MCU_FILES	+= $(wildcard plat/$(ARCH)/cmn/*_help.S)
MCU_FILES	+= $(wildcard plat/$(ARCH)/cmn/*_help.c)

# Add startup file
ifneq ($(wildcard plat/$(ARCH)/$(CHIP_LOWER_CASE)/startup.S),)
MCU_FILES	+= plat/$(ARCH)/$(CHIP_LOWER_CASE)/startup.S
else
MCU_FILES	+= plat/$(ARCH)/cmn/startup.S
endif

# Core driver files
MCU_FILES	+= \
	source/main.c \
	driver/spinlock.S \
	driver/cmd_utils.c \
	driver/shell.c \
	driver/print.c \
	driver/console/console.c \
	driver/wdt.c \
	driver/delay.c \
	driver/gpio.c \
	driver/idc.c \
	driver/rct.c \
	driver/cmvi_pwr.c

# Include paths
MCU_INCLUDES	:= \
	-Iinclude \
	-Iplat/$(ARCH) \
	-Iplat/$(ARCH)/cmn \
	-IFreeRTOS/include \
	-IFreeRTOS/portable/GCC/ARM_$(ARCH) \
	-I$(MCU_EXTRA_INC)

# Console configuration
ifeq ("$(CONFIG_$(ARCH)_LOG_CONSOLE)","y")
ifeq ("$(CONFIG_$(ARCH)_CMVI_LOG_CONSOLE)","y")
MCU_CFLAGS	+= -DCMVI_LOG_CONSOLE
else
MCU_LOG_CONSOLE := \
	$(shell grep ^CONFIG_$(ARCH)_UART_LOG_CONSOLE $(DOT_CONFIG) \
	| sed -e s/^CONFIG_$(ARCH)_UART_LOG_CONSOLE=//)
MCU_CFLAGS	+= -DUART_LOG_CONSOLE=$(MCU_LOG_CONSOLE)
endif
endif

# Board-specific files
ifneq ($(wildcard $(AMB_BOARD_DIR)/bsp/bsp_$(ARCH_LOWER_CASE).c),)
MCU_FILES	+= $(AMB_BOARD_DIR)/bsp/bsp_$(ARCH_LOWER_CASE).c
endif

# Optional SPI support
ifeq ("$(CONFIG_$(ARCH)_SPI)","y")
MCU_CFLAGS	+= -DCONFIG_$(ARCH)_SPI
MCU_FILES	+= driver/spi.c
endif

# Optional RPMSG support
ifeq ("$(CONFIG_$(ARCH)_RPMSG)","y")

RPMSG_LITE_FILES	:= \
	rpmsg-lite/lib/common/llist.c \
	rpmsg-lite/lib/rpmsg_lite/porting/environment/rpmsg_env_freertos.c \
	rpmsg-lite/lib/rpmsg_lite/porting/platform/$(CHIP_LOWER_CASE)/rpmsg_platform.c \
	rpmsg-lite/lib/rpmsg_lite/rpmsg_lite.c \
	rpmsg-lite/lib/rpmsg_lite/rpmsg_ns.c \
	rpmsg-lite/lib/rpmsg_lite/rpmsg_queue.c \
	rpmsg-lite/lib/virtio/virtqueue.c \
	rpmsg-lite/tests/test_$(ARCH_LOWER_CASE)_rpmsg.c

RPMSG_LITE_INCS	:= \
	-Irpmsg-lite/lib/include \
	-Irpmsg-lite/lib/include/environment/freertos \
	-Irpmsg-lite/lib/include/platform/$(CHIP_LOWER_CASE)

MCU_REMAP_SIZE	:= $(shell grep ^CONFIG_$(ARCH)_REMAP_SIZE $(DOT_CONFIG))
MCU_REMAP_ADDRESS:= $(shell grep ^CONFIG_$(ARCH)_REMAP_ADDRESS $(DOT_CONFIG))
MCU_CFLAGS	+= -DCONFIG_$(ARCH)_RPMSG \
		-D$(strip $(MCU_REMAP_SIZE)) \
		-D$(strip $(MCU_REMAP_ADDRESS))
MCU_INCLUDES	+= $(RPMSG_LITE_INCS)
MCU_FILES	+= $(RPMSG_LITE_FILES) driver/rpmsg.c
endif

# Image header Marker 8 bytes len: 0x03 or 0x52
MCU_MARKER	:= $(MCU_OUTPUT)/$(ARCH)_HDR.marker

# Build objects
MCU_OBJS	:= $(patsubst %.c, $(MCU_OUTPUT)/%.o, $(filter %.c, $(MCU_FILES)))
MCU_OBJS	+= $(patsubst %.S, $(MCU_OUTPUT)/%.o, $(filter %.S, $(MCU_FILES)))
MCU_DEPS	:= $(MCU_OBJS:.o=.d)

# Build targets
all: $(TARGET_BIN) $(TARGET_HDR_BIN)

$(TARGET_BIN):$(TARGET_ELF)
	@$(MCU_OBJCOPY) --gap-fill=0xff -O binary --strip-all $< $@

$(TARGET_HDR_BIN):$(TARGET_BIN) $(MCU_MARKER)
	@cat $(MCU_MARKER) $(TARGET_BIN) > $@

$(TARGET_ELF): $(MCU_OBJS) $(MCU_LINKER)
	@echo "Linking $(TARGET)..."
	@$(MCU_CC) -Wl,--start-group $(MCU_OBJS) -Wl,--end-group $(MCU_LDFLAGS) -o $@
	@$(MCU_NM) -n $@ | grep -v '\( [aUw] \)|\(__crc__\)' > $(subst .elf,.map,$@)
	@$(MCU_OBJDUMP) -D $@ > $(subst .elf,.elf.dump,$@)
	@echo "Build Summary for $(TARGET_ELF):"
	@echo "--------------------------------------------------"
	@$(MCU_SZ) --format=berkeley $@
	@echo "--------------------------------------------------"
	@$(MCU_SZ) -A $@
	@echo "--------------------------------------------------"

$(MCU_LINKER):$(MCU_LINKER_SRC)
	@echo "CPP $<"
	@mkdir -p $(dir $@)
	@$(MCU_CPP) -D__ASM__ $(MCU_CFLAGS) $(MCU_INCLUDES) $< | sed /^#/d > $@

# Generate the header marker for the image
$(MCU_OUTPUT)/CM3_HDR.marker:
	@echo "Generating CM3_HDR.marker"
	@mkdir -p $(dir $@)
	@/bin/bash -c "printf '\x03%.0s' {1..8} > $@"

$(MCU_OUTPUT)/R52_HDR.marker:
	@echo "Generating R52_HDR.marker"
	@mkdir -p $(dir $@)
	@/bin/bash -c "printf '\x52%.0s' {1..8} > $@"


# Compilation rules
$(MCU_OUTPUT)/%.o: %.c
	@echo "CC $< "
	@mkdir -p $(dir $@)
	@$(MCU_CC) $(MCU_CFLAGS) $(MCU_INCLUDES) -c $< -o $@

$(MCU_OUTPUT)/%.o: $(AMB_BOARD_DIR)/bsp/%.c
	@echo "CC $< "
	@mkdir -p $(dir $@)
	@$(MCU_CC) $(MCU_CFLAGS) $(MCU_INCLUDES) -c $< -o $@

$(MCU_OUTPUT)/%.o: %.S
	@echo "AS $< "
	@mkdir -p $(dir $@)
	@$(MCU_CC) $(MCU_AFLAGS) $(MCU_INCLUDES) -c $< -o $@

$(MCU_OUTPUT)/%.o: $(AMB_BOARD_DIR)/bsp/%.S
	@echo "AS $< "
	@mkdir -p $(dir $@)
	@$(MCU_CC) $(MCU_AFLAGS) $(MCU_INCLUDES) -c $< -o $@

# Dependency generation rules
$(MCU_OUTPUT)/%.d: %.c
	@mkdir -p ${dir $@}
	@set -e; rm -f $@; $(MCU_CC) -MM $< $(MCU_INCLUDES) $(MCU_CFLAGS) > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(addprefix $(dir $@),\1.o):,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(MCU_OUTPUT)/%.d: %.S
	@mkdir -p ${dir $@}
	@set -e; rm -f $@; $(MCU_CC) -MM $< $(MCU_INCLUDES) $(MCU_CFLAGS) > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(addprefix $(dir $@),\1.o):,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(MCU_OUTPUT)/%.d: $(AMB_BOARD_DIR)/bsp/%.c
	@mkdir -p ${dir $@}
	@set -e; rm -f $@; $(MCU_CC) -MM $< $(MCU_INCLUDES) $(MCU_CFLAGS) > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(addprefix $(dir $@),\1.o):,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(MCU_OUTPUT)/%.d: $(AMB_BOARD_DIR)/bsp/%.S
	@mkdir -p ${dir $@}
	@set -e; rm -f $@; $(MCU_CC) -MM $< $(MCU_INCLUDES) $(MCU_CFLAGS) > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(addprefix $(dir $@),\1.o):,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Include dependencies unless cleaning
ifneq ($(MAKECMDGOALS),clean)
-include $(MCU_DEPS)
endif

# Prevent make from removing intermediate files
.SECONDARY:

# Clean target
clean:
	@-rm -rf $(MCU_OUTPUT) $(TARGET)
	@echo "Clean $(ARCH) complete!"

# Install the firmware to the target
# proc use the original bin and prboot use the header version
# header: 8bytes of 0x03 or 0x52
# struct header {
# 	uint8_t header[8];
# }
install:
ifneq ("$(CONFIG_BOOT_$(ARCH))","y")
	@install -D $(TARGET_BIN) $(ENV_INS_ROOT)/lib/firmware/rproc-ambarella-rproc-fw
endif
	@install -D $(TARGET_HDR_BIN) $(ENV_IMG_ROOT)/$(ARCH)_IMG.bin
	@install -D $(TARGET_BIN) $(ENV_IMG_ROOT)/$(ARCH).bin
	@install -D $(TARGET_BIN) $(ENV_IMG_ROOT)/$(ARCH_LOWER_CASE).bin

