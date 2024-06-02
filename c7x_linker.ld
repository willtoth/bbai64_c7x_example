#define C7X_DMA_DATA_START      0xAA000000
#define C7X_ALLOCATED_START     0xAA100000

#define C7X_EXT_D_LENGTH        0x00100000
#define C7X_IPC_D_LENGTH        0x00100000
#define C7X_TEXT_LENGTH         0x00100000
#define C7X_DDR_SPACE_LENGTH    0x00C00000

#define C7X_EXT_D_START         C7X_ALLOCATED_START
#define C7X_IPC_D_START         C7X_EXT_D_START + C7X_EXT_D_LENGTH
#define C7X_TEXT_START          C7X_IPC_D_START + C7X_IPC_D_LENGTH
#define C7X_DDR_SPACE_START     C7X_TEXT_START  + C7X_TEXT_LENGTH

MEMORY
{
  C7X_EXT_D           (RWIX)	: ORIGIN = C7X_EXT_D_START        LENGTH = C7X_EXT_D_LENGTH
  C7X_IPC_D           (RWIX)	: ORIGIN = C7X_IPC_D_START        LENGTH = C7X_IPC_D_LENGTH
  C7X_TEXT            (RWIX)	: ORIGIN = C7X_TEXT_START         LENGTH = C7X_TEXT_LENGTH
  C7X_DDR_SPACE       (RWIX)	: ORIGIN = C7X_DDR_SPACE_START    LENGTH = C7X_DDR_SPACE_LENGTH
  L2SRAM              (RWX)     : ORIGIN = 0x64800000,            LENGTH = 0x00070000
}

--ram_model
-heap  0x20000
-stack 0x20000
--args 0x1000
--diag_suppress=10068 /* "no matching section" */
--cinit_compression=off

SECTIONS
{
    boot:
    {
      boot.*<boot.oe71>(.text)
    } load > C7X_TEXT ALIGN(0x200000)
    .vecs       >       C7X_DDR_SPACE ALIGN(0x400000)
    .secure_vecs >      C7X_DDR_SPACE ALIGN(0x200000)
    .text:_c_int00 > C7X_DDR_SPACE ALIGN(0x200000)
    .text       >       C7X_DDR_SPACE ALIGN(0x200000)

    .bss        >       C7X_DDR_SPACE  /* Zero-initialized data */
    .data       >       C7X_DDR_SPACE  /* Initialized data */

    .cinit      >       C7X_DDR_SPACE  /* could be part of const */
    .init_array >       C7X_DDR_SPACE  /* C++ initializations */
    .stack      >       C7X_DDR_SPACE  ALIGN(0x20000) /* Needed for Nested ISR handling */
    .args       >       C7X_DDR_SPACE
    .cio        >       C7X_DDR_SPACE
    .const      >       C7X_DDR_SPACE
    .switch     >       C7X_DDR_SPACE /* For exception handling. */
    .sysmem     >       C7X_DDR_SPACE /* heap */

    GROUP:              >  C7X_DDR_SPACE
    {
        .data.Mmu_tableArray          : type=NOINIT
        .data.Mmu_tableArraySlot      : type=NOINIT
        .data.Mmu_level1Table         : type=NOINIT
        .data.Mmu_tableArray_NS       : type=NOINIT
        .data.Mmu_tableArraySlot_NS   : type=NOINIT
        .data.Mmu_level1Table_NS      : type=NOINIT
    }

    .log_shared_mem: { 
        *(.log_shared_mem*)
    } > C7X_IPC_D

    .resource_table: {
        *(.resource_table*)
    } > C7X_EXT_D
}
