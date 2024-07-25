# Include the core environment definitions; this will set $(TOP).
include ../../py/mkenv.mk

# Include py core make definitions.
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

BOARD ?= virt

CROSS_COMPILE ?= riscv64-unknown-elf-

GCC_VERSION = $(word 1, $(subst ., , $(shell $(CC) -dumpversion)))

ifeq ($(BOARD),virt)
ABI = ilp32
# GCC 10 and lower do not recognise the Zicsr extension in the
# architecture name.  "Make" unfortunately does not provide any simple way
# to perform numeric comparisons, so to keep things simple we assume that
# GCC is at least version 10 for the time being.
ifeq ($(GCC_VERSION),10)
ARCH ?= rv32imac
else
# Recent GCC versions explicitly require to declare extensions.
ARCH ?= rv32imac_zicsr
ARCH_LD ?= rv32imac_zicsr
endif
AFLAGS = -mabi=$(ABI) -march=$(ARCH)
BAREMETAL = -nostartfiles
CFLAGS += $(AFLAGS) $(BAREMETAL)
CFLAGS += -DQEMU_SOC_VIRT
ARCH_LD ?= $(ARCH)
LDSCRIPT = virt.ld
LDFLAGS += $(BAREMETAL) -mabi=$(ABI) -march=$(ARCH_LD) -T $(LDSCRIPT) -Wl,-EL
SRC_BOARD_O = shared/runtime/gchelper_native.o shared/runtime/gchelper_rv32i.o
SRC_BOARD_O += entrypoint.o
endif

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -g
AFLAGS += -g
COPT = -O0
else
COPT += -Os -DNDEBUG
endif

# Set CFLAGS and libraries.
CFLAGS += $(COPT) -I. -I$(BUILD) -I$(TOP)
LIBS += -lm

# Define the required source files.
SRC_C = \
	main.c \
	mphalport.c \
	uart.c \
	shared/readline/readline.c \
	shared/runtime/gchelper_generic.c \
	shared/runtime/pyexec.c \
	shared/runtime/stdout_helpers.c \

SRC_ASM = \
	entrypoint.s \

# Define source files containung qstrs.
SRC_QSTR += shared/readline/readline.c shared/runtime/pyexec.c

# Define the required object files.
OBJ = $(PY_CORE_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_ASM:.s=.o))

# Define the top-level target, the main firmware.
all: $(BUILD)/firmware.elf

# `make debug` will block QEMU until a debugger is connected to port 1234.
debug: $(BUILD)/firmware.elf
	qemu-system-riscv32 -machine $(BOARD) -bios none $(QEMU_EXTRA) -nographic -monitor null -semihosting -serial mon:stdio -S -s -kernel $<

run: $(BUILD)/firmware.elf
	qemu-system-riscv32 -machine $(BOARD) -bios none $(QEMU_EXTRA) -nographic -monitor null -semihosting -serial mon:stdio -kernel $<

# Define how to build the firmware.
$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

# Include remaining core make rules.
include $(TOP)/py/mkrules.mk