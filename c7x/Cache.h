/*
 *  Copyright (C) 2018-2023 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ======== GENERATED SECTIONS ========
 *
 *     PROLOGUE
 *     INCLUDES
 *
 *     INTERNAL DEFINITIONS
 *     MODULE-WIDE CONFIGS
 *     VIRTUAL FUNCTIONS
 *     FUNCTION DECLARATIONS
 *     CONVERTORS
 *     SYSTEM FUNCTIONS
 *
 *     EPILOGUE
 *     STATE STRUCTURES
 *     PREFIX ALIASES
 */


/*
 * ======== PROLOGUE ========
 */

#ifndef Cache__include
#define Cache__include


/*
 * ======== INCLUDES ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <c7x.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Cache_Size Cache_Size;
typedef struct Cache_Module_State Cache_Module_State;

/* Type */
enum Cache_Type {
    Cache_Type_L1P  = 0x1,
    Cache_Type_L1D  = 0x2,
    Cache_Type_L1   = 0x3,
    Cache_Type_L2P  = 0x4,
    Cache_Type_L2D  = 0x8,
    Cache_Type_L2   = 0xC,
    Cache_Type_ALLP = 0x5,
    Cache_Type_ALLD = 0xA,
    Cache_Type_ALL  = 0x7fff
};
typedef enum Cache_Type Cache_Type;


/*
 * ======== AUXILIARY DEFINITIONS ========
 */

/* Mode */
enum Cache_Mode {
    Cache_Mode_FREEZE,
    Cache_Mode_BYPASS,
    Cache_Mode_NORMAL
};
typedef enum Cache_Mode Cache_Mode;

/* L1Size */
enum Cache_L1Size {
    Cache_L1Size_0K  = 0U,
    Cache_L1Size_4K  = 1U,
    Cache_L1Size_8K  = 2U,
    Cache_L1Size_16K = 3U,
    Cache_L1Size_32K = 4U
};
typedef enum Cache_L1Size Cache_L1Size;

/* L2Size */
enum Cache_L2Size {
    Cache_L2Size_0K    = 0U,
    Cache_L2Size_32K   = 1U,
    Cache_L2Size_64K   = 2U,
    Cache_L2Size_128K  = 3U,
    Cache_L2Size_256K  = 4U,
    Cache_L2Size_512K  = 5U,
    Cache_L2Size_1024K = 6U
};
typedef enum Cache_L2Size Cache_L2Size;

/* PC */
#define Cache_PC (1)

/* WTE */
#define Cache_WTE (2)

/* PCX */
#define Cache_PCX (4)

/* PFX */
#define Cache_PFX (8)

/* Size */
struct Cache_Size {
    Cache_L1Size l1pSize;
    Cache_L1Size l1dSize;
    Cache_L2Size l2Size;
};


/* enableCache */
extern const bool Cache_enableCache;

/* initSize */
extern const Cache_Size Cache_initSize;

/* atomicBlockSize */
extern const uint32_t Cache_atomicBlockSize;




/*
 * ======== FUNCTION DECLARATIONS ========
 */


/* enable */
void Cache_enable( uint16_t type);

/* inv */
void Cache_inv(void * blockPtr, size_t byteCnt, uint16_t type, bool wait);

/* wb */
void Cache_wb(void * blockPtr, size_t byteCnt, uint16_t type, bool wait);

/* wbInv */
void Cache_wbInv(void * blockPtr, size_t byteCnt, uint16_t type, bool wait);

/* wait */
void Cache_wait( void);

/* disable */
void Cache_disable( uint16_t type);

/* getSize */
void Cache_getSize( Cache_Size *size);

/* setSize */
void Cache_setSize( Cache_Size *size);

/* wbAll */
void Cache_wbAll( void);

/* wbL1dAll */
void Cache_wbL1dAll( void);

/* wbInvAll */
void Cache_wbInvAll( void);

/* wbInvL1dAll */
void Cache_wbInvL1dAll( void);

/* setL2CFG */
void Cache_setL2CFG( unsigned int size);

/* getL2CFG */
unsigned long Cache_getL2CFG( void);

/* setL1DCFG */
void Cache_setL1DCFG( unsigned int size);

/* getL1DCFG */
unsigned long Cache_getL1DCFG( void);

/* setL2WB */
void Cache_setL2WB( unsigned int flag);

/* setL2WBINV */
void Cache_setL2WBINV( unsigned int flag);

/* setL1DWB */
void Cache_setL1DWB( unsigned int flag);

/* setL1DWBINV */
void Cache_setL1DWBINV( unsigned int flag);

/* startup */
void Cache_startup( void);


void Cache_Module_startup( void );



/*
 * ======== STATE STRUCTURES ========
 */



/* Module_State */
struct Cache_Module_State {
    uint64_t L1DCFG;
    uint64_t L2CFG;
};

/* Module__state__V */
extern struct Cache_Module_State Cache_Module_state;






#ifdef __cplusplus
    } /* extern C */
#endif

#endif

