SRCDIR += ./c7x/src
INCDIR += ./c7x
SRCS_ASM_COMMON_INTERNAL += Cache_asm.asm Clobber_asm.asm Exception_asm.asm Hwi_asm.asm
SRCS_ASM_COMMON_INTERNAL += Hwi_asm_switch.asm Hwi_disp_always.asm Mmu_asm.asm
SRCS_COMMON_INTERNAL += boot.c c7x_module_config.c Cache.c Exception.c Hwi.c IntrinsicsSupport.c Mmu.c
SRCS_COMMON_INTERNAL += Mmu_table.c Startup.c
SRCS_COMMON = $(addprefix $(SRCDIR)/,$(SRCS_COMMON_INTERNAL))
SRCS_ASM_COMMON = $(addprefix $(SRCDIR)/,$(SRCS_ASM_COMMON_INTERNAL))
CFLAGS_LOCAL_COMMON += -DHwi_bootToNonSecure__D=true
CFLAGS_LOCAL_COMMON += -DException_vectors__D