/*
 * Copyright (c) 2016-2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== Mmu.c ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <c7x.h>
#include <stdio.h>

#include <ti/osal/DebugP.h>

#include "Hwi.h"
#include "Cache.h"


#include "Mmu.h"

#define Mmu_PADDR_MASK          0x0000FFFFFFFFFFFF

#define Mmu_GRANULE_SIZE_4KB    0x000
#define Mmu_GRANULE_SIZE_16KB   0x100
#define Mmu_GRANULE_SIZE_64KB   0x080

#define Mmu_OUTER_SHAREABLE     0x0400
#define Mmu_OUTER_CACHEABLE     0x2000
#define Mmu_INNER_CACHEABLE     0x0800

#define Mmu_MEMTYPE_CACHEABLE   0x08000

#define Mmu_WALK_EN             0x1

extern uint64_t *Mmu_level1Table;

extern uint64_t  Mmu_tableArray[];
extern uint64_t  Mmu_tableArraySlot;

#if Hwi_bootToNonSecure__D

#define Mmu_level1Table_NS    Mmu_level1Table_NS
#define Mmu_tableArray_NS     Mmu_tableArray_NS
#define Mmu_tableArraySlot_NS Mmu_tableArraySlot_NS

extern uint64_t *Mmu_level1Table_NS;
extern uint64_t  Mmu_tableArray_NS[];
extern uint64_t  Mmu_tableArraySlot_NS;

#endif

/*
 * TODO Pending work
 * =================
 *  - Add custom memset() for BIOS (name it differently). Copy implementation
 *    from RTS version.
 */

/*
 *  ======== Mmu_addBlockEntry ========
 */
void Mmu_addBlockEntry(uint8_t level, uint64_t *tablePtr, uint16_t tableIdx,
    uint64_t paddr, Mmu_MapAttrs *mapAttrs)
{
    uint64_t desc;

    if (level == 3) {
        desc = 0x3;
    }
    else {
        desc = Mmu_DescriptorType_BLOCK;
    }

    desc |= ((uint64_t)(mapAttrs->attrIndx & 0x7) << 2) |
            ((uint64_t)(mapAttrs->ns << 5)) |
            ((uint64_t)(mapAttrs->accessPerm & 0x3) << 6) |
            ((uint64_t)(mapAttrs->shareable & 0x3) << 8) |
            ((uint64_t)(0x1) << 10) |  /* access flag */
            ((uint64_t)(!(mapAttrs->global) & 0x1) << 11) |
            ((uint64_t)(paddr & ~((1 << Mmu_configInfo.tableOffset[level]) - 1))) |
            ((uint64_t)(!(mapAttrs->privExecute) & 0x1) << 53) |
            ((uint64_t)(!(mapAttrs->userExecute) & 0x1) << 54);

    tablePtr[tableIdx] = desc;
}

/*
 *  ======== Mmu_addTableEntry ========
 */
uint64_t* Mmu_addTableEntry(uint64_t *tablePtr, uint16_t tableIdx,
    Mmu_MapAttrs *mapAttrs, bool secure)
{
    uint64_t desc, *newTable;

    newTable = Mmu_allocTable(secure);
    if (newTable == NULL) {
        return (NULL);
    }

    desc = ((uint64_t)Mmu_DescriptorType_TABLE & 0x3) |
           ((uint64_t)newTable & ~(Mmu_granuleSize - 1));
    tablePtr[tableIdx] = desc;

    return (newTable);
}

/*
 *  ======== Mmu_allocTable ========
 */
uint64_t* Mmu_allocTable(bool secure)
{
    uint64_t *table;
    unsigned int i, tableLen = (Mmu_granuleSize >> 3);
    uint64_t *slot;

    if (secure) {
        table = &Mmu_tableArray[tableLen * Mmu_tableArraySlot];
        slot = &Mmu_tableArraySlot;
    }
#if Hwi_bootToNonSecure__D
    else {
        table = &Mmu_tableArray_NS[tableLen * Mmu_tableArraySlot_NS];
        slot = &Mmu_tableArraySlot_NS;
    }
#else
    else {
        return (NULL);
    }
#endif

    if (*slot == (~0)) {
        return (NULL);
    }

    *slot = *table;

    /* Zero-out level 1 table */
    for (i = 0; i < tableLen; i++) {
        /*
         * Use (i << 2) instead of 0 to init table, in order to force
         * compiler to not use memset() as an optimization
         */
        table[i] = (i << 2);
    }

    return (table);
}

/*
 *  ======== Mmu_disable ========
 *  Function to disable the MMU.
 */
void Mmu_disable()
{
    uint16_t type;
    unsigned int   key;

    /* if MMU is alreay disabled, just return */
    if (!(Mmu_isEnabled())) {
        return;
    }

    key = Hwi_disable();

    /* get the current enabled bits */
//    type = Cache_getEnabled();
    type = Cache_Type_L1D;

    if (type & Cache_Type_L1D) {
        /* disable the L1 data cache */
        Cache_disable(Cache_Type_L1D);
    }

    /* disables the MMU */
    Mmu_disableI();

    /* Invalidate entire TLB */
    Mmu_tlbInvAll(0);

    /* set cache back to initial settings */
    Cache_enable(type);

    Hwi_restore(key);
}

/*
 *  ======== Mmu_disableI ========
 */
void Mmu_disableI()
{
    asm("\t mvk64.l1    0xfffffffffffffffeLL, a2 \n"
        "\t mvc.s1      SCR, a3                  \n"
        "\t nop                                  \n"
        "\t andd.l1     a2, a3, a3               \n"
        "\t mvc.s1      a3, SCR");
}

/*
 *  ======== Mmu_enable ========
 */
void Mmu_enable()
{
    unsigned int   key;
    unsigned int   mode;

    /* if MMU is already enabled then just return */
    if (Mmu_isEnabled()) {
        return;
    }

    key = Hwi_disable();

    Cache_disable(Cache_Type_L1D);

    /* Invalidate entire TLB */
    // EDIT: This line reboots the core
    //Mmu_tlbInvAll(0);

    /* enables the MMU */
    mode = Hwi_getCXM();
    if (mode == Hwi_TSR_CXM_SecureSupervisor) {
        Mmu_enableI_secure();
    }
    Mmu_enableI();

    Cache_enable(Cache_Type_L1D);

    Hwi_restore(key);
}

/*
 *  ======== Mmu_enableI_secure ========
 */
void Mmu_enableI_secure()
{
    asm("\t mvk64.l1    0x80000000000000C1LL, a2 \n"
        "\t mvc.s1      SCR, a3                  \n"
        "\t nop                                  \n"
        "\t ord.l1      a2, a3, a3               \n"
        "\t mvc.s1      a3, SCR                  \n"
        "\t mvc.s1      a3, SCR_S");
}

/*
 *  ======== Mmu_enableI ========
 */
void Mmu_enableI()
{
    asm("\t mvk64.l1    0x80000000000000C1LL, a2 \n"
        "\t mvc.s1      SCR, a3                \n"
        "\t nop                                  \n"
        "\t ord.l1      a2, a3, a3               \n"
        "\t mvc.s1      a3, SCR");
}

/*
 *  ======== Mmu_freeTable ========
 */
void Mmu_freeTable(uint64_t *table)
{
    *table = Mmu_tableArraySlot;
    Mmu_tableArraySlot = (table - Mmu_tableArray) / (Mmu_granuleSize >> 3);
}

/*
 *  ======== Mmu_initMapAttrs ========
 */
void Mmu_initMapAttrs(Mmu_MapAttrs *attrs)
{
    /* Assert that attrs != NULL */
    DebugP_assert(attrs != NULL);

    attrs->ns = Mmu_defaultMapAttrs.ns;
    attrs->accessPerm = Mmu_defaultMapAttrs.accessPerm;
    attrs->privExecute = Mmu_defaultMapAttrs.privExecute;
    attrs->userExecute = Mmu_defaultMapAttrs.userExecute;
    attrs->shareable = Mmu_defaultMapAttrs.shareable;
    attrs->attrIndx = Mmu_defaultMapAttrs.attrIndx;
    attrs->global = Mmu_defaultMapAttrs.global;
}

/*
 *  ======== Mmu_map ========
 */
bool Mmu_map(uint64_t vaddr, uint64_t paddr, size_t size, Mmu_MapAttrs *mapAttrs, bool secure)
{
    unsigned int key;
    uint64_t *tablePtr;
    bool retStatus, enabled;

    /* Assert that mapAttrs != NULL */
    DebugP_assert(mapAttrs != NULL);

    /* Range check on vaddr and paddr */
    DebugP_assert(vaddr <= Mmu_PADDR_MASK);
    DebugP_assert(paddr <= Mmu_PADDR_MASK);

    /* Alignment check on vaddr, paddr & size */
    DebugP_assert((vaddr & (Mmu_granuleSize - 1)) == 0);
    DebugP_assert((paddr & (Mmu_granuleSize - 1)) == 0);
    DebugP_assert((size & (Mmu_granuleSize - 1)) == 0);

    key = Hwi_disable();

    /* determine the current state of the MMU */
    enabled = Mmu_isEnabled();

    /* disable the MMU (if already disabled, does nothing) */
    Mmu_disable();

    if (secure) {
        tablePtr = Mmu_level1Table;
    }
#if Hwi_bootToNonSecure__D
    else {
        tablePtr = Mmu_level1Table_NS;
    }
#else
    else {
        return (false);
    }
#endif

    if (Mmu_configInfo.noLevel0Table) {
        retStatus = Mmu_tableWalk(1, tablePtr, &vaddr, &paddr,
            &size, mapAttrs, secure);
    }
    else {
        retStatus = Mmu_tableWalk(0, tablePtr, &vaddr, &paddr,
            &size, mapAttrs, secure);
    }

    /* Ensure all translation table updates have been observed */
#if 0
    __asm__ __volatile__ (
        "dsb sy"
        ::: "memory"
    );
#endif

    if (enabled) {
        /* if Mmu was enabled, then re-enable it */
        Mmu_enable();
    }

    Hwi_restore(key);

    return (retStatus);
}

/*
 *  ======== Mmu_readBlockEntry ========
 */
void Mmu_readBlockEntry(uint8_t level, uint64_t *tablePtr, uint16_t tableIdx,
    uint64_t *paddr, Mmu_MapAttrs *mapAttrs)
{
    uint64_t desc;

    desc = tablePtr[tableIdx];

    mapAttrs->attrIndx = (Mmu_AttrIndx)((desc >> 2) & 0x7);
    mapAttrs->ns = (desc >> 5) & 0x1;
    mapAttrs->accessPerm = (Mmu_AccessPerm)((desc >> 6) & 0x3);
    mapAttrs->shareable = (Mmu_Shareable)((desc >> 8) & 0x3);
    mapAttrs->global = !((desc >> 11) & 0x1);
    mapAttrs->privExecute = !((desc >> 53) & 0x1);
    mapAttrs->userExecute = !((desc >> 54) & 0x1);

    *paddr = desc & (uint64_t)Mmu_PADDR_MASK &
        ~((1 << Mmu_configInfo.tableOffset[level]) - 1);
}

/*
 *  ======== Mmu_setMAIR ========
 */
void Mmu_setMAIR(uint8_t idx, uint8_t attr)
{
    unsigned int key;

    key = Hwi_disable();

    /* Write attr to MAIRn register */
    Mmu_setMAIRAsm(idx, attr);

    /* Invalidate the entire TLB (broadcast to other cores) */
    Mmu_tlbInvAll(0);

    Hwi_restore(key);
}

/* --> InitMmu */
void vInitMmu( void ) {}

/* initFunc */
#pragma DATA_SECTION(Mmu_initFunc, ".const:Mmu_initFunc");
const Mmu_InitFuncPtr Mmu_initFunc = ((Mmu_InitFuncPtr)(vInitMmu));



/*
 *  ======== Mmu_startup ========
 */
void Mmu_startup()
{
    uint64_t tcr = 0;
    unsigned int i, tableLen = Mmu_configInfo.tableLength;
    unsigned int mode;

    /* Initialize MAIR */
    Mmu_setMAIRAsm(0, Mmu_MAIR0);
    Mmu_setMAIRAsm(1, Mmu_MAIR1);
    Mmu_setMAIRAsm(2, Mmu_MAIR2);
    Mmu_setMAIRAsm(3, Mmu_MAIR3);
    Mmu_setMAIRAsm(4, Mmu_MAIR4);
    Mmu_setMAIRAsm(5, Mmu_MAIR5);
    Mmu_setMAIRAsm(6, Mmu_MAIR6);
    Mmu_setMAIRAsm(7, Mmu_MAIR7);

    if (Mmu_granuleSize == Mmu_GranuleSize_4KB) {
        tcr = Mmu_GRANULE_SIZE_4KB;
    }
    else if (Mmu_granuleSize == Mmu_GranuleSize_16KB) {
        tcr = Mmu_GRANULE_SIZE_16KB;
    }
    else if (Mmu_granuleSize == Mmu_GranuleSize_64KB) {
        tcr = Mmu_GRANULE_SIZE_64KB;
    }

    /*
     * TCR config:
     *  - MMU tables reside in outer and inner write-back write-allocate
     *    cacheable memory
     *  - Physical address size is 48-bits wide
     *  - TTBR0 is used for translating VA 0 to (2^48 - 1)
     */
    tcr = tcr | (uint64_t)Mmu_MEMTYPE_CACHEABLE | Mmu_OUTER_SHAREABLE |
          Mmu_OUTER_CACHEABLE | Mmu_INNER_CACHEABLE |
          ((64 - Mmu_PA_MAX_WIDTH) << 1) | Mmu_WALK_EN;

    mode = Hwi_getCXM();
    if (mode == Hwi_TSR_CXM_SecureSupervisor) {
        Mmu_setTCR(tcr, 1);
    }
    Mmu_setTCR(tcr, 0);

    /*
     *  When running in SMP mode, the MMU table init should be done
     *  by core 0 (master) only.
     *
     *  If not running in SMP mode, Core_getId() always returns 0,
     *  and the below init code will be run.
     */
//    if (Core_getId() == 0) {
        /* Initialize table array */
        for (i = 0; i < Mmu_tableArrayLen; i++) {
            Mmu_tableArray[tableLen * i] = i + 1;
#if Hwi_bootToNonSecure__D
            Mmu_tableArray_NS[tableLen * i] = i + 1;
#endif
        }
        Mmu_tableArray[tableLen * (i - 1)] = (~0);
        Mmu_tableArraySlot = 0;

        /* Allocate level1 Table */
        Mmu_level1Table = Mmu_allocTable(1);

#if Hwi_bootToNonSecure__D
        Mmu_tableArray_NS[tableLen * (i - 1)] = (~0);
        Mmu_tableArraySlot_NS = 0;

        /* Allocate level1 Table */
        Mmu_level1Table_NS = Mmu_allocTable(0);
#endif
//    }

    /* Install MMU translation tables */
    if (mode == Hwi_TSR_CXM_SecureSupervisor) {
        Mmu_init(Mmu_level1Table, 1);
    }

#if Hwi_bootToNonSecure__D
    Mmu_init(Mmu_level1Table_NS, 0);
#endif

    /*
     * Call init function. This function is part of the application and will
     * add MMU mappings. If in SMP mode, core 0 has already done this.
     */
//    if (Core_getId() == 0) {
        Mmu_initFunc();
//    }

    /* Invalidate entire TLB */
//    Mmu_tlbInvAll(0);

    if (Mmu_enableMMU) {
        mode = Hwi_getCXM();
        if (mode == Hwi_TSR_CXM_SecureSupervisor) {
            // This line hangs the core
            Mmu_enableI_secure();
        }
        else {
            Mmu_enableI();
        }
    }

    if (Hwi_bootToNonSecure) {
        Hwi_secureStart();
    }
}

/*
 *  ======== Mmu_tableWalk ========
 */
bool Mmu_tableWalk(uint8_t level, uint64_t *tablePtr, uint64_t *vaddr, uint64_t *paddr,
    size_t *size, Mmu_MapAttrs *mapAttrs, bool secure)
{
    uint64_t desc;
    uint16_t tableIdx;
    size_t  blockSize;
    bool   retStatus;
    bool   blockTranslation;
    uint64_t *nextLevelTablePtr;

    blockTranslation = true;
    blockSize = 1 << Mmu_configInfo.tableOffset[level];
    if ((level == 0) ||
       ((level == 1) && (Mmu_granuleSize != Mmu_GranuleSize_4KB))) {
        blockTranslation = false;
    }

    tableIdx = (*vaddr >> Mmu_configInfo.tableOffset[level]) &
        Mmu_configInfo.indexMask;

    while ((*size != 0) && (tableIdx < Mmu_configInfo.tableLength)) {
        desc = tablePtr[tableIdx];

        if (((desc & 0x3) == Mmu_DescriptorType_TABLE) && (level != 3)) {
            if ((blockTranslation == true) && (*size >= blockSize) &&
                (*vaddr & (blockSize - 1) == 0)) {
                Mmu_addBlockEntry(level, tablePtr, tableIdx, *paddr, mapAttrs);
                *size = *size - blockSize;
                *vaddr = *vaddr + blockSize;
                *paddr = *paddr + blockSize;
                Mmu_freeTable((uint64_t *)(Mmu_PADDR_MASK & desc &
                    ~(uint64_t)(Mmu_granuleSize - 1)));
            }
            else {
                nextLevelTablePtr = (uint64_t *)(Mmu_PADDR_MASK & desc &
                    ~(uint64_t)(Mmu_granuleSize - 1));
                retStatus = Mmu_tableWalk(level + 1, nextLevelTablePtr,
                    vaddr, paddr, size, mapAttrs, secure);
                if (!retStatus) {
                    return (false);
                }
            }
        }
        else if (((desc & 0x3) != Mmu_DescriptorType_TABLE) || (level == 3)) {
            if ((level == 0) || ((level < 3) && (*size < blockSize)) ||
               ((*size >= blockSize) && ((*vaddr & (blockSize - 1)) != 0))) {
                uint64_t vaddrCopy = (*vaddr & (~(blockSize - 1)));
                uint64_t paddrCopy;
                Mmu_MapAttrs mapAttrsCopy;
                size_t sizeCopy = blockSize;

                if ((desc & 0x3) == Mmu_DescriptorType_BLOCK) {
                    Mmu_readBlockEntry(level, tablePtr, tableIdx, &paddrCopy,
                        &mapAttrsCopy);
                }

                nextLevelTablePtr =
                    Mmu_addTableEntry(tablePtr, tableIdx, mapAttrs, secure);
                if (nextLevelTablePtr == NULL) {
                    return (false);
                }

                if ((desc & 0x3) == Mmu_DescriptorType_BLOCK) {
                    /*
                     * If old entry is a block entry, a new table entry is
                     * added and merged with the old block entry.
                     */
                    Mmu_tableWalk(level + 1, nextLevelTablePtr, &vaddrCopy,
                        &paddrCopy, &sizeCopy, &mapAttrsCopy, secure);
                }

                retStatus = Mmu_tableWalk(level + 1, nextLevelTablePtr,
                    vaddr, paddr, size, mapAttrs, secure);
                if (!retStatus) {
                    return (false);
                }
            }
            else if ((blockTranslation == true) && (*size >= blockSize)) {
                Mmu_addBlockEntry(level, tablePtr, tableIdx, *paddr, mapAttrs);
                *size = *size - blockSize;
                *vaddr = *vaddr + blockSize;
                *paddr = *paddr + blockSize;
            }
        }

        tableIdx++;
    }

    return (true);
}

/*
 *  ======== Mmu_initFuncDefault ========
 */
void Mmu_initFuncDefault()
{
    bool ret;
    Mmu_MapAttrs attrs;

    Mmu_initMapAttrs(&attrs);

    /*
     * Setup secure mappings
     */

    attrs.attrIndx = Mmu_AttrIndx_MAIR0;
    attrs.ns = 0;
    attrs.privExecute = 1;

    ret = Mmu_map(0x02400000, 0x02400000, 0x000c0000, &attrs, 1); /* dmtimer */
    if (!ret) {
        goto fail;
    }

    ret = Mmu_map(0x02800000, 0x02800000, 0x00001000, &attrs, 1); /* uart    */
    if (!ret) {
        goto fail;
    }

    ret = Mmu_map(0x78000000, 0x78000000, 0x08000000, &attrs, 1); /* clec    */
    if (!ret) {
        goto fail;
    }

    attrs.attrIndx = Mmu_AttrIndx_MAIR7;

    ret = Mmu_map(0x60000000, 0x60000000, 0x18000000, &attrs, 1); /* msmc    */
    if (!ret) {
        goto fail;
    }

    ret = Mmu_map(0x80000000, 0x80000000, 0x00800000, &attrs, 1); /* ddr     */
    if (!ret) {
        goto fail;
    }

    /*
     * Setup non-secure mappings
     */
#if Hwi_bootToNonSecure__D
    attrs.attrIndx = Mmu_AttrIndx_MAIR0;
    attrs.ns = 1;

    ret = Mmu_map(0x02400000, 0x02400000, 0x000c0000, &attrs, 0); /* dmtimer */
    if (!ret) {
        goto fail;
    }

    ret = Mmu_map(0x02800000, 0x02800000, 0x00001000, &attrs, 0); /* uart    */
    if (!ret) {
        goto fail;
    }

    ret = Mmu_map(0x78000000, 0x78000000, 0x08000000, &attrs, 0); /* clec    */
    if (!ret) {
        goto fail;
    }

    attrs.attrIndx = Mmu_AttrIndx_MAIR7;

    ret = Mmu_map(0x60000000, 0x60000000, 0x18000000, &attrs, 0); /* msmc    */
    if (!ret) {
        goto fail;
    }

    /* Secure code that transitions to non-secure */
    ret = Mmu_map(0x80000000, 0x80000000, 0x00800000, &attrs, 0); /* ddr     */
    if (!ret) {
        goto fail;
    }
#endif
    return;

fail:
    DebugP_assert((bool)false);
}

#if defined(__GNUC__) && !defined(__ti__)
#error "Unsupported compiler"
#else



#pragma DATA_SECTION(Mmu_tableArraySlot, ".data.Mmu_tableArraySlot");
#pragma DATA_ALIGN(Mmu_tableArraySlot, 4096);
uint64_t Mmu_tableArraySlot;
#pragma DATA_SECTION(Mmu_tableArraySlot_NS, ".data.Mmu_tableArraySlot_NS");
#pragma DATA_ALIGN(Mmu_tableArraySlot_NS, 4096);
uint64_t Mmu_tableArraySlot_NS;

#pragma DATA_SECTION(Mmu_level1Table, ".data.Mmu_level1Table");
#pragma DATA_ALIGN(Mmu_level1Table, 4096);
uint64_t* Mmu_level1Table;
#pragma DATA_SECTION(Mmu_level1Table_NS, ".data.Mmu_level1Table_NS");
#pragma DATA_ALIGN(Mmu_level1Table_NS, 4096);
uint64_t* Mmu_level1Table_NS;

#endif

/*
 * ======== ti.sysbios.family.c7x.Mmu INITIALIZERS ========
 */

/* defaultMapAttrs */
#pragma DATA_SECTION(Mmu_defaultMapAttrs, ".const:Mmu_defaultMapAttrs");
const Mmu_MapAttrs Mmu_defaultMapAttrs = {
    1,  /* ns */
    Mmu_AccessPerm_PRIV_RW_USER_NONE,  /* accessPerm */
    1,  /* privExecute */
    0,  /* userExecute */
    Mmu_Shareable_OUTER,  /* shareable */
    Mmu_AttrIndx_MAIR0,  /* attrIndx */
    1,  /* global */
};

/* enableMMU */
#pragma DATA_SECTION(Mmu_enableMMU, ".const:Mmu_enableMMU");
const bool Mmu_enableMMU = 0; // EDIT: Set to 0, something is not set up right, and initialization fails on startup

/* granuleSize */
#pragma DATA_SECTION(Mmu_granuleSize, ".const:Mmu_granuleSize");
const Mmu_GranuleSize Mmu_granuleSize = Mmu_GranuleSize_4KB;

/* MAIR0 */
#pragma DATA_SECTION(Mmu_MAIR0, ".const:Mmu_MAIR0");
const uint8_t Mmu_MAIR0 = (uint8_t)0x0U;

/* MAIR1 */
#pragma DATA_SECTION(Mmu_MAIR1, ".const:Mmu_MAIR1");
const uint8_t Mmu_MAIR1 = (uint8_t)0x4U;

/* MAIR2 */
#pragma DATA_SECTION(Mmu_MAIR2, ".const:Mmu_MAIR2");
const uint8_t Mmu_MAIR2 = (uint8_t)0x8U;

/* MAIR3 */
#pragma DATA_SECTION(Mmu_MAIR3, ".const:Mmu_MAIR3");
const uint8_t Mmu_MAIR3 = (uint8_t)0xcU;

/* MAIR4 */
#pragma DATA_SECTION(Mmu_MAIR4, ".const:Mmu_MAIR4");
const uint8_t Mmu_MAIR4 = (uint8_t)0x44U;

/* MAIR5 */
#pragma DATA_SECTION(Mmu_MAIR5, ".const:Mmu_MAIR5");
const uint8_t Mmu_MAIR5 = (uint8_t)0x4fU;

/* MAIR6 */
#pragma DATA_SECTION(Mmu_MAIR6, ".const:Mmu_MAIR6");
const uint8_t Mmu_MAIR6 = (uint8_t)0xbbU;

/* MAIR7 */
#pragma DATA_SECTION(Mmu_MAIR7, ".const:Mmu_MAIR7");
const uint8_t Mmu_MAIR7 = (uint8_t)0x7dU;

/* tableArrayLen */
#pragma DATA_SECTION(Mmu_tableArrayLen, ".const:Mmu_tableArrayLen");
const unsigned int Mmu_tableArrayLen = (unsigned int)0x10U;

/* configInfo */
#pragma DATA_SECTION(Mmu_configInfo, ".const:Mmu_configInfo");
const Mmu_ConfigInfo Mmu_configInfo = {
    (uint64_t)0x1ffU,  /* indexMask */
    (uint32_t)0x200U,  /* tableLength */
    {
        (uint8_t)0x27U,  /* [0] */
        (uint8_t)0x1eU,  /* [1] */
        (uint8_t)0x15U,  /* [2] */
        (uint8_t)0xcU,  /* [3] */
    },  /* tableOffset */
    (uint8_t)0xcU,  /* granuleSizeBits */
    (uint8_t)0x9U,  /* indexBits */
    0,  /* noLevel0Table */
};

