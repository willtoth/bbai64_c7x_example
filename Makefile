APP ?= main.elf
APP_SOURCES ?= main.c dhry_1.c dhry_2.c c7x_mmu.c linpack.c

TI_CGT_C7x ?= /home/will/ti-cgt-c7000_4.1.0.LTS

.PHONY: $(APP)

C7x_CC ?= $(TI_CGT_C7x)/bin/cl7x
# C7x_SIZE ?= $(CROSS_COMPILE)size
C7x_OBJDUMP ?= $(TI_CGT_C7x)/bin/dis7x

-include ./c7x/src_files_c7x.mk

INCLUDE_ALL = $(INCDIR) $(TI_CGT_C7x)/include
INCLUDES = $(addprefix -I,$(INCLUDE_ALL))

# Taken from PDK
CFLAGS_INTERNAL = --silicon_version=7100 --abi=eabi -q -mo -pden -pds=238 -pds=880 -pds1110 --endian=little -O4 --opt_for_speed=5
LNKFLAGS_INTERNAL_COMMON = -q --warn_sections --diag_suppress=10063 --ram_model

all: $(APP)

clean:
	rm -f $(APP) *.obj $(APP).lst

$(APP): $(APP_SOURCES) c7x_linker.ld
	$(C7x_CC) $(CFLAGS_INTERNAL) $(APP_SOURCES) $(SRCS_COMMON) $(SRCS_ASM_COMMON) $(CFLAGS_LOCAL_COMMON) -O4 $(INCLUDES) --run_linker $(LNKFLAGS_INTERNAL_COMMON) --library=c7x_linker.ld --search_path=$(TI_CGT_C7x)/lib --output_file=$(APP)
	# $(CROSS_SIZE) $(APP)
	$(C7x_OBJDUMP) --all $(APP) > $(APP).lst
	# sudo cp $(APP) /lib/firmware/
	# sudo echo stop > /sys/class/remoteproc/remoteproc14/state
	# sudo echo $(APP) > /sys/class/remoteproc/remoteproc14/firmware
	# sudo echo start > /sys/class/remoteproc/remoteproc14/state
	# sudo cat /sys/kernel/debug/remoteproc/remoteproc14/trace0
