APP ?= main.elf
APP_SOURCES ?= main.c dhry_1.c dhry_2.c

TI_CGT_C7x ?= ~/ti-cgt-c7000_4.1.0.LTS

.PHONY: $(APP)

C7x_CC ?= $(TI_CGT_C7x)/bin/cl7x
# C7x_SIZE ?= $(CROSS_COMPILE)size
C7x_OBJDUMP ?= $(TI_CGT_C7x)/bin/dis7x

all: $(APP)

clean:
	rm -f $(APP) *.obj $(APP).lst

$(APP): $(APP_SOURCES) c7x_linker.ld
	$(C7x_CC) $(APP_SOURCES) $(CFLAGS) -O4 -I=$(TI_CGT_C7x)/include --run_linker --library=c7x_linker.ld --search_path=$(TI_CGT_C7x)/lib --output_file=$(APP)
	# $(CROSS_SIZE) $(APP)
	$(C7x_OBJDUMP) --all $(APP) > $(APP).lst
	# sudo cp $(APP) /lib/firmware/
	# sudo echo stop > /sys/class/remoteproc/remoteproc14/state
	# sudo echo $(APP) > /sys/class/remoteproc/remoteproc14/firmware
	# sudo echo start > /sys/class/remoteproc/remoteproc14/state
	# sudo cat /sys/kernel/debug/remoteproc/remoteproc14/trace0
