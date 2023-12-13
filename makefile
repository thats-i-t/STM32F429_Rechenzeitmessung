################################################################################
# HINTS:
# $@	Name of the BIN (file being generated)
# $<	Name of the first Prerequisite (usually the source file)
# $^	Name of all prerequisites
#
# EXAMPLES
#
################################################################################

# TARGET can be STM32F40_41xxx (for STM32F407) or STM32F429_439xx
TARGET = STM32F40_41xxx

################################################################################
# PROJECT
################################################################################
ADDRESS = 0x08000000
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
BASE_DIR = $(current_dir)
OUTPUT_DIR = $(BASE_DIR)/output
OBJ_DIR = $(OUTPUT_DIR)/obj

################################################################################
# TOOLS
################################################################################
TOOL_DIR = $(BASE_DIR)/tools
# COMPILERPATH = $(TOOL_DIR)/gcc-arm-none-eabi-7-2018-q2-update-win32\bin
COMPILERPATH = $(TOOL_DIR)/gcc-arm-none-eabi-10.3-2021.10/bin
ST-FLASH-PATH = $(TOOL_DIR)/stlink-1.3.0-win64/bin
BIN_2_DFU = $(TOOL_DIR)/dfu-20101118/dfu
DFU_UTIL_PATH = $(TOOL_DIR)/dfu-util-0.9-win64

################################################################################
# EXECUTABLES
################################################################################
BIN = $(OUTPUT_DIR)/main.bin
ELF = $(OUTPUT_DIR)/main.elf
HEX = $(OUTPUT_DIR)/main.hex
DFU = $(OUTPUT_DIR)/main.dfu
MAP_FILE = $(OUTPUT_DIR)/memory.map
A2L = $(OUTPUT_DIR)/main.a2l

################################################################################
# PROJECT-INDEPENDENT SOURCES
################################################################################
CODE_BASE_PATH = $(BASE_DIR)/code
CODE_SRC_DIR = $(CODE_BASE_PATH)/src
CODE_INC_DIR = $(CODE_BASE_PATH)/inc

CODE_SRC_FILES = $(wildcard $(CODE_SRC_DIR)/*.c)
CODE_OBJ_FILES = $(patsubst $(CODE_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CODE_SRC_FILES))

################################################################################
# CMSIS
################################################################################

CMSIS_BASE_PATH = $(BASE_DIR)/ext/cmsis

# COMMONS
CMSIS_INC_DIR = $(CMSIS_BASE_PATH)/common/inc

# BOOT
CMSIS_BOOT_BASE_PATH = $(CMSIS_BASE_PATH)/cmsis_boot
CMSIS_BOOT_SRC_DIR = $(CMSIS_BOOT_BASE_PATH)/src
CMSIS_BOOT_INC_DIR = $(CMSIS_BOOT_BASE_PATH)/inc

CMSIS_BOOT_SRC_FILES = $(wildcard $(CMSIS_BOOT_SRC_DIR)/*.c)
CMSIS_BOOT_OBJ_FILES = $(patsubst $(CMSIS_BOOT_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CMSIS_BOOT_SRC_FILES))

# LIB
CMSIS_LIB_BASE_PATH = $(CMSIS_BASE_PATH)/cmsis_lib
CMSIS_LIB_SRC_DIR = $(CMSIS_LIB_BASE_PATH)/source
CMSIS_LIB_INC_DIR = $(CMSIS_LIB_BASE_PATH)/include

CMSIS_LIB_SRC_FILES = $(wildcard $(CMSIS_LIB_SRC_DIR)/*.c)
CMSIS_LIB_OBJ_FILES = $(patsubst $(CMSIS_LIB_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CMSIS_LIB_SRC_FILES))


ifeq ($(TARGET),STM32F429_439xx)

# DISPLAY
CMSIS_DISPLAY_BASE_PATH = $(CMSIS_BASE_PATH)/display
CMSIS_DISPLAY_SRC_DIR = $(CMSIS_DISPLAY_BASE_PATH)/src
CMSIS_DISPLAY_INC_DIR = $(CMSIS_DISPLAY_BASE_PATH)/inc

CMSIS_DISPLAY_SRC_FILES = $(wildcard $(CMSIS_DISPLAY_SRC_DIR)/*.c)
CMSIS_DISPLAY_OBJ_FILES = $(patsubst $(CMSIS_DISPLAY_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CMSIS_DISPLAY_SRC_FILES))

endif


################################################################################
# USB + VCP SOURCES
################################################################################
ifeq ($(TARGET),STM32F40_41xxx)

USB_BASE_PATH = $(BASE_DIR)/ext/usb_stm32f407
VCP_BASE_PATH = $(BASE_DIR)/ext/vcp_stm32f407

else ifeq ($(TARGET),STM32F429_439xx)

USB_BASE_PATH = $(BASE_DIR)/ext/usb_cdc_dev
VCP_BASE_PATH = 

endif

USB_SRC_DIR = $(USB_BASE_PATH)/src
USB_INC_DIR = $(USB_BASE_PATH)/include
USB_SRC_FILES = $(wildcard $(USB_SRC_DIR)/*.c)
USB_OBJ_FILES = $(patsubst $(USB_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(USB_SRC_FILES))

VCP_SRC_DIR = $(VCP_BASE_PATH)/src
VCP_INC_DIR = $(VCP_BASE_PATH)/include
VCP_SRC_FILES = $(wildcard $(VCP_SRC_DIR)/*.c)
VCP_OBJ_FILES = $(patsubst $(VCP_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(VCP_SRC_FILES))

################################################################################
# SEPARATELY COMPILED LIB
################################################################################
EXT_LIB_PATH = $(BASE_DIR)/ext
EXT_LIB = RefCalc_STM32

################################################################################
# LISTS WITH ALL SOURCES / INCLUDES / OBJECTS
################################################################################

SRC_DIRS =     $(CODE_SRC_DIR)   $(CMSIS_LIB_SRC_DIR)   $(CMSIS_BOOT_SRC_DIR) 	$(USB_SRC_DIR)
SRC_FILES =    $(CODE_SRC_FILES) $(CMSIS_LIB_SRC_FILES) $(CMSIS_BOOT_SRC_FILES) $(USB_SRC_FILES)
OBJ_FILES =    $(CODE_OBJ_FILES) $(CMSIS_LIB_OBJ_FILES) $(CMSIS_BOOT_OBJ_FILES) $(USB_OBJ_FILES)
INCLUDE_DIRS = $(CODE_INC_DIR)   $(CMSIS_LIB_INC_DIR)   $(CMSIS_BOOT_INC_DIR) 	$(USB_INC_DIR)     

ifeq ($(TARGET),STM32F429_439xx)

SRC_DIRS = $(CMSIS_DISPLAY_SRC_DIR)  
SRC_FILES = $(CMSIS_DISPLAY_SRC_FILES)
OBJ_FILES = $(CMSIS_DISPLAY_OBJ_FILES)
INCLUDE_DIRS = $(CMSIS_DISPLAY_INC_DIR) 

endif

ifeq ($(TARGET),STM32F40_41xxx)

SRC_DIRS +=     $(VCP_SRC_DIR)
SRC_FILES +=    $(VCP_SRC_FILES)
OBJ_FILES +=    $(VCP_OBJ_FILES)
INCLUDE_DIRS += $(VCP_INC_DIR)

endif

INCLUDE_DIRS += $(CMSIS_INC_DIR)
INCLUDES = $(patsubst %, -I%, $(INCLUDE_DIRS))

DEP_FILES := $(OBJ_FILES:%.o=%.d)

################################################################################
# DO NOT EDIT BELOW THIS LINE
################################################################################

CC = $(COMPILERPATH)/arm-none-eabi-gcc
CP = $(COMPILERPATH)/arm-none-eabi-objcopy
OD = $(COMPILERPATH)/arm-none-eabi-objdump
CSIZE = $(COMPILERPATH)/arm-none-eabi-size

DEFS = -DUSE_STDPERIPH_DRIVER -D$(TARGET) -DSTM32F4XX -DSTM32F4xx -DSTM32F4 -DHSE_VALUE=8000000 -DHSI_VALUE=16000000 \
-D__FPU_USED -DUSE_EMBEDDED_PHY -DUSE_USB_OTG_FS -D__ASSEMBLY__ -D__FPU_PRESENT -D__VFP_FP__ -DUSE_DEVICE_MODE

#ADD_DEFS = -DFP_DT=0 
ADD_DEFS =

# -fsingle-precision-constant -mfloat-abi=softfp
MCFLAGS = -Wall --specs=nosys.specs -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fdata-sections \
-ffunction-sections -g

MCFLAGS += -Wno-strict-aliasing # USB lib has some bad pointer dereferencing :( (but still works)
MCFLAGS += -Wno-attributes # otherwise in STM32F407-USB-Lib: warning: 'packed' attribute ignored for type 'uint32_t *'

OPTIMIZE = -O0 # -O0 -O3

DEPFLAGS = -MD -MP -MF $(OBJ_DIR)/$*.d

CFLAGS = $(MCFLAGS) $(DEPFLAGS) $(OPTIMIZE) $(DEFS) $(ADD_DEFS) $(INCLUDES)

LFLAGS = -Xlinker -Map=$(MAP_FILE) -Tlinkerscript.ld -g -L -nostartfiles -lm #--gc-sections

################################################################################
# Compiler command
# - Remove the @ at the beginning of the line to see the output of the CC cmd
################################################################################
define CC-COMMAND
@$(info )
@$(info Making target [${@}] ...)
@$(info )
$(CC) -c $(CFLAGS) $< -o $@
endef

################################################################################
# TARGETS
################################################################################

# first target is default build target
default_target = all
.PHONY: default
default:
	@echo Default build target is: $(default_target)
	make $(default_target)

################################################################################
.PHONY: flash
flash: all
	@$(ST-FLASH-PATH)/st-flash write $(BIN) 0x8000000

################################################################################
# Works only if device is in DFU Mode?
# bin specifies instead of DFU file... correct?
################################################################################
.PHONY: flash2
flash2:
	$(DFU_UTIL_PATH)/dfu-util 0483:DF11 -a0 -s 0x08000000:leave -D $(BIN)

################################################################################
# DFU file needed for flashing via DFU?
################################################################################
.PHONY: dfu
dfu: $(DFU)

$(DFU): all
	@echo Making dfu from bin [$@]
	@$(BIN_2_DFU) -b $(ADDRESS):$(BIN) --device=0x0483:0xDF11 $(DFU)

################################################################################
.PHONY: all
all: delete_build_info $(ELF) $(HEX) $(BIN)

################################################################################
$(HEX): $(ELF)
	@echo Making hex [$@]
	@$(CP) --change-address $(ADDRESS) -O ihex $(ELF) $(HEX) 
	
################################################################################
$(BIN): $(ELF)
	@echo Making binary [$@]
	@$(CP) -O binary $^ $@

################################################################################
$(ELF): $(OBJ_FILES)
	@echo Linking [$@]
	@$(CC) $(CFLAGS) $^ -L$(EXT_LIB_PATH) -l$(EXT_LIB) -o $@ $(LFLAGS) -Wl,--gc-sections
	@$(CSIZE) $@

################################################################################
# Targets for object files in different folders
################################################################################

$(OBJ_DIR)/%.o: $(CODE_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)

$(OBJ_DIR)/%.o: $(CMSIS_BOOT_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)	
	
$(OBJ_DIR)/%.o: $(CMSIS_LIB_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)
	
$(OBJ_DIR)/%.o: $(CMSIS_DISPLAY_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)
	
$(OBJ_DIR)/%.o: $(USB_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)
	
$(OBJ_DIR)/%.o: $(VCP_SRC_DIR)/%.c $(mkfile_path) | $(OBJ_DIR)
	$(CC-COMMAND)



# $(OBJ_DIR)/%.o: $(CODE_SRC_DIR)/%.c $(OBJ_DIR)/%.d | $(OBJ_DIR)#specifying dependency file not needed?

################################################################################
# SINGLE RULE FOR ALL OBJECTS DOES NOT WORK YET :-(
################################################################################
# $(patsubst %,$(OBJ_DIR)/%,$(notdir $(patsubst %.c,%.o,$(SRC_FILES)))): $(SRC_FILES) | $(OBJ_DIR)
# 	@$(CC-COMMAND)

################################################################################
.PHONY: objects
objects: delete_build_info $(OBJ_FILES)

################################################################################
# Output directories
################################################################################
$(OUTPUT_DIR):
	@echo Creating output dir...
	@cmd /E:ON /C mkdir $(subst /,\,$(OUTPUT_DIR))

$(OBJ_DIR): | $(OUTPUT_DIR)
	@echo Creating object dir...
	@cmd /E:ON /C mkdir $(subst /,\,$(OBJ_DIR))

################################################################################
.PHONY: delete_build_info
delete_build_info:
	@echo Deleting diagnostics.o so that compile date and time will be up-to-date.
	@rm -f $(OBJ_DIR)/diagnostics.o

################################################################################
.PHONY: clean
clean:
	@echo "Cleaning up whole project."
	@rm -r $(OUTPUT_DIR)

################################################################################
.PHONY: rebuild
rebuild: clean all

################################################################################
tmp = $(patsubst %,$(OBJ_DIR)/%,$(notdir $(patsubst %.c,%.o,$(SRC_FILES))))
tmp = $(DEP_FILES)

.PHONY: test
test:# Target for testing purposes
	@echo Makefile path: $(mkfile_path)
#	@echo usb src files: $(USB_SRC_FILES)
#	@echo proj src files: $(PROJECT_SRC_FILES)
	@echo matlab src dir $(PROJ_MATLAB_SRC_DIR)
#	@echo matlab src files: $(PROJ_MATLAB_SRC_FILES)
#	@echo matlab obj files: $(PROJ_MATLAB_OBJ_FILES)
#	@echo tmp variable: $(tmp)


################################################################################
# Include-Dependecies :: This must come after the initial, default target; otherwise the included dependency files will abscond with your default target setting
-include $(DEP_FILES) 
