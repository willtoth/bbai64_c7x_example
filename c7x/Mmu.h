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
 * ======== PROLOGUE ========
 */

#ifndef Mmu__include
#define Mmu__include

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

typedef struct Mmu_MapAttrs Mmu_MapAttrs;
typedef struct Mmu_ConfigInfo Mmu_ConfigInfo;
typedef struct Mmu_Module_State Mmu_Module_State;

/*
 * ======== AUXILIARY DEFINITIONS ========
 */

/* PA_MAX_WIDTH */
#define Mmu_PA_MAX_WIDTH (48)

/* PA_SIZE_ENCODING */
#define Mmu_PA_SIZE_ENCODING (0x5)

/* AttrIndx */
enum Mmu_AttrIndx {
    Mmu_AttrIndx_MAIR0 = 0,
    Mmu_AttrIndx_MAIR1,
    Mmu_AttrIndx_MAIR2,
    Mmu_AttrIndx_MAIR3,
    Mmu_AttrIndx_MAIR4,
    Mmu_AttrIndx_MAIR5,
    Mmu_AttrIndx_MAIR6,
    Mmu_AttrIndx_MAIR7
};
typedef enum Mmu_AttrIndx Mmu_AttrIndx;

/* DescriptorType */
enum Mmu_DescriptorType {
    Mmu_DescriptorType_INVALID0 = 0,
    Mmu_DescriptorType_BLOCK = 1,
    Mmu_DescriptorType_INVALID1 = 2,
    Mmu_DescriptorType_TABLE = 3
};
typedef enum Mmu_DescriptorType Mmu_DescriptorType;

/* GranuleSize */
enum Mmu_GranuleSize {
    Mmu_GranuleSize_4KB = 0x1000,
    Mmu_GranuleSize_16KB = 0x4000,
    Mmu_GranuleSize_64KB = 0x10000
};
typedef enum Mmu_GranuleSize Mmu_GranuleSize;

/* Shareable */
enum Mmu_Shareable {
    Mmu_Shareable_NONE = 0x0,
    Mmu_Shareable_OUTER = 0x2,
    Mmu_Shareable_INNER = 0x3
};
typedef enum Mmu_Shareable Mmu_Shareable;

/* AccessPerm */
enum Mmu_AccessPerm {
    Mmu_AccessPerm_PRIV_RW_USER_NONE = 0x0,
    Mmu_AccessPerm_PRIV_RW_USER_RW = 0x1,
    Mmu_AccessPerm_PRIV_RO_USER_NONE = 0x2,
    Mmu_AccessPerm_PRIV_RO_USER_RO = 0x3
};
typedef enum Mmu_AccessPerm Mmu_AccessPerm;

/* InitFuncPtr */
typedef void (*Mmu_InitFuncPtr)(void );

/* MapAttrs */
struct Mmu_MapAttrs {
    bool ns;
    Mmu_AccessPerm accessPerm;
    bool privExecute;
    bool userExecute;
    Mmu_Shareable shareable;
    Mmu_AttrIndx attrIndx;
    bool global;
};


/*
 * ======== INTERNAL DEFINITIONS ========
 */

/* ConfigInfo */
typedef uint8_t Mmu_ConfigInfo_tableOffset[4];
struct Mmu_ConfigInfo {
    uint64_t indexMask;
    uint32_t tableLength;
    Mmu_ConfigInfo_tableOffset tableOffset;
    uint8_t granuleSizeBits;
    uint8_t indexBits;
    bool noLevel0Table;
};



/* defaultMapAttrs */
extern const Mmu_MapAttrs Mmu_defaultMapAttrs;

/* enableMMU */
extern const bool Mmu_enableMMU;

/* granuleSize */
extern const Mmu_GranuleSize Mmu_granuleSize;

/* MAIR0 */
extern const uint8_t Mmu_MAIR0;

/* MAIR1 */
extern const uint8_t Mmu_MAIR1;

/* MAIR2 */
extern const uint8_t Mmu_MAIR2;

/* MAIR3 */
extern const uint8_t Mmu_MAIR3;

/* MAIR4 */
extern const uint8_t Mmu_MAIR4;

/* MAIR5 */
extern const uint8_t Mmu_MAIR5;

/* MAIR6 */
extern const uint8_t Mmu_MAIR6;

/* MAIR7 */
extern const uint8_t Mmu_MAIR7;


/* initFunc */
extern const Mmu_InitFuncPtr Mmu_initFunc;

/* tableArrayLen */
extern const unsigned int Mmu_tableArrayLen;


/* configInfo */
extern const Mmu_ConfigInfo Mmu_configInfo;



/*
 * ======== FUNCTION DECLARATIONS ========
 */


/* enable */
void Mmu_enable( void);

/* initMapAttrs */
void Mmu_initMapAttrs( Mmu_MapAttrs *descAttrs);

/* isEnabled */
bool Mmu_isEnabled( void);

/* map */
bool Mmu_map( uint64_t vaddr, uint64_t paddr, size_t size, Mmu_MapAttrs *attrs, bool secure);

/* setMAIR */
void Mmu_setMAIR( uint8_t attrIndx, uint8_t attr);

/* startup */
void Mmu_startup( void);

/* tlbInvAll */
void Mmu_tlbInvAll( uint64_t type);

/* initFuncDefault */
void Mmu_initFuncDefault( void);

/* disable */
void Mmu_disable( void);

/* disableI */
void Mmu_disableI( void);

/* enableI */
void Mmu_enableI( void);

/* enableI_secure */
void Mmu_enableI_secure( void);

/* addBlockEntry */
void Mmu_addBlockEntry( uint8_t level, uint64_t *tablePtr, uint16_t tableIdx, uint64_t paddr, Mmu_MapAttrs *attrs);

/* addTableEntry */
uint64_t *Mmu_addTableEntry( uint64_t *tablePtr, uint16_t tableIdx, Mmu_MapAttrs *attrs, bool secure);

/* allocTable */
uint64_t *Mmu_allocTable( bool secure);

/* freeTable */
void Mmu_freeTable( uint64_t *table);

/* init */
void Mmu_init( void * tableAddr, bool secure);

/* readBlockEntry */
void Mmu_readBlockEntry( uint8_t level, uint64_t *tablePtr, uint16_t tableIdx, uint64_t *paddr, Mmu_MapAttrs *attrs);

/* setMAIRAsm */
void Mmu_setMAIRAsm( uint8_t attrIndx, uint8_t attr);

/* tableWalk */
bool Mmu_tableWalk( uint8_t level, uint64_t *tablePtr, uint64_t *vaddr, uint64_t *paddr, size_t *size, Mmu_MapAttrs *attrs, bool secure);

/* setTCR */
void Mmu_setTCR( uint64_t regVal, bool secure);






/*
 * ======== STATE STRUCTURES ========
 */






#ifdef __cplusplus
    } /* extern C */
#endif

#endif
