1. cortex-M3 use AHB-UART0 as console.The Amboot-Code should also init AHB-UART0 for debug used.
    if the platform has CMVI then switch to use CMVI console.
2. Amboot Load the cortex-M3 code(maxcode Size 32KB and run.
3. (For CV72/CV3AD685): LoadAddress is 0xFFE0040000 and run from this address.
4. (For CV75)LoadAddress is  0xFFE0050000 and relocated its code to CMVI SRAM.
5. R52 load at dram and will relocated the code to ATCM and run in ATCM.

=================================================================================
bringup branch debug used Makefile example:

ifeq ($(CONFIG_BOOT_CM3), y)
.PHONY: cortex-m3 clean_cortex-m3

CM3_DEVELOP_FLAGS=\
	MCU_EXTRA_INC=$(shell pwd) \
	MCU_OUTPUT=$(AMB_BOARD_OUT)/boot/freertos/CM3 \
	ARCH=CM3

CM3_PRIVATE_PATH:=$(BOOT_DIR)/freertos
cortex-m3: $(DOT_CONFIG)
	@bear --output CM3.json -- $(MAKE) -s $(CM3_DEVELOP_FLAGS) -C $(CM3_PRIVATE_PATH) all
	@echo "Build $@ Done ..."

clean_cortex-m3: $(DOT_CONFIG)
	@$(MAKE) -s $(CM3_DEVELOP_FLAGS) -C $(CM3_PRIVATE_PATH) clean
	@echo " $@ Done ..."

CM3_IMAGE:=$(AMB_BOARD_OUT)/boot/freertos/CM3/CM3.bin
export CM3_IMAGE

amboot: cortex-m3
endif

ifeq ($(CONFIG_BOOT_R52), y)
.PHONY: cortex-r52 clean_cortex-r52

R52_DEVELOP_FLAGS=\
	MCU_EXTRA_INC=$(shell pwd) \
	MCU_OUTPUT=$(AMB_BOARD_OUT)/boot/freertos/R52 \
	ARCH=R52 \
	BOOT_ADDR=$(KERNEL_RAM_START)

R52_PRIVATE_PATH:=$(BOOT_DIR)/freertos
cortex-r52: $(DOT_CONFIG)
	@bear --output R52.json -- $(MAKE) -s $(R52_DEVELOP_FLAGS) -C $(R52_PRIVATE_PATH) all
	@echo "Build $@ Done ..."

clean_cortex-r52: $(DOT_CONFIG)
	@$(MAKE) -s $(R52_DEVELOP_FLAGS) -C $(R52_PRIVATE_PATH) clean
	@echo " $@ Done ..."

R52_IMAGE:=$(AMB_BOARD_OUT)/boot/freertos/R52/R52.bin
export R52_IMAGE

amboot: cortex-r52
endif

# for freertos. need add to build/Makefile
FREERTOS_BIN_DIR:=$(patsubst %/,%, $(dir $(BUILD_DIR)))
CM3_BIN			:= $(FREERTOS_BIN_DIR)/CM3/CM3.bin
R52_BIN			:= $(FREERTOS_BIN_DIR)/R52/R52.bin

install:
	@mkdir -p $(BIN_DIR)
	@$(if $(BST_IMAGE),ln -rfs $(BUILD_DIR)/$(BST_IMAGE) $(BIN_DIR)/bst.bin)
	@$(if $(PRB_IMAGE),ln -rfs $(BUILD_DIR)/$(PRB_IMAGE) $(BIN_DIR)/prb.bin)
	@$(if $(BLD_IMAGE),ln -rfs $(BUILD_DIR)/$(BLD_IMAGE) $(BIN_DIR)/bld.bin)
	@$(if $(ATF_IMAGE),ln -rfs $(BUILD_DIR)/$(ATF_IMAGE) $(BIN_DIR)/atf.bin)
	@$(if $(FIP_IMAGE),ln -rfs $(BUILD_DIR)/$(FIP_IMAGE) $(BIN_DIR)/fip.bin)
	@$(if $(DTB_BIN),  ln -rfs $(BUILD_DIR)/$(DTB_BIN)   $(BIN_DIR)/dtb.bin)
	@$(if $(ENV_BIN),  ln -rfs $(BUILD_DIR)/$(ENV_BIN)   $(BIN_DIR)/env.bin)
	@$(if $(FWPROG),   ln -rfs $(BUILD_DIR)/$(FWPROG)    $(BIN_DIR)/fwp.bin)
	@$(if $(CM3_BIN),  ln -rfs $(CM3_BIN) $(BIN_DIR)/CM3.bin)
	@$(if $(R52_BIN),  ln -rfs $(R52_BIN) $(BIN_DIR)/R52.bin)
	@mkdir -p $(IMG_DIR) && rm -rf $(IMG_DIR)/*
	@$(if $(wildcard memfwprog/*.elf), mv -f memfwprog/*.elf $(IMG_DIR))
	@$(if $(wildcard memfwprog/*.bin), mv -f memfwprog/*.bin $(IMG_DIR))
===================================================================================

