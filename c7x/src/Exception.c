/*
 * Copyright (c) 2014-2021 Texas Instruments Incorporated
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
 *  ======== Exception.c ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "Hwi.h"

#include "Exception.h"
#pragma FUNC_EXT_CALLED(Exception_handler);

#include <c7x.h>

#include <ti/osal/DebugP.h>

/* Object__sizingError */
typedef char Exception_Context__sizingError[(sizeof(Exception_Context) > Exception_sizeContextBuf) ? -1 : 1];


/*
 *  ======== Exception_Module_startup ========
 *  Exception_Module_startup must be called to enable events to cause
 *  an exception.
 */
void Exception_Module_startup(void)
{
    /*
     *  Exception uses Hwi, needs to wait for its startup.
     */
    if (Exception_useInternalBuffer) {
        Exception_Module_state.excPtr = (char *)
            (((uint64_t)Exception_Module_state.contextBuf +
                Exception_sizeContextBuf - 8) & ~0x7);
    }
    else {
        Exception_Module_state.excPtr = Hwi_getIsrStackAddress();
    }

    return;
}

#pragma FUNC_CANNOT_INLINE(printTwoVectors)
static void printTwoVectors(char * name1, void * vectorReg1[],
                            char * name2, void * vectorReg2[])
{
    int32_t i;

    DebugP_exceptionLog("\n%s=0x%x [0]\n", (uintptr_t)name1, (uintptr_t)vectorReg1[0]);
	DebugP_exceptionLog("\n%s=0x%x [0]\n", (uintptr_t)name2, (uintptr_t)vectorReg2[0]);
    for (i = 1; i < 8; i++) {
        DebugP_exceptionLog("0x%x [%d]\n", (uintptr_t)vectorReg1[i], i);
    }
	for (i = 1; i < 8; i++) {
        DebugP_exceptionLog("0x%x [%d]\n", (uintptr_t)vectorReg2[i], i);
    }
}

/*
 *  ======== Exception_handler ========
 *  This handler is called by the dispatch function.  It calls the various
 *  exception hook functions.
 */
void Exception_handler(bool abortFlag, int vectorType)
{
    uint64_t *contextStack;
    uint64_t nrp;
    uint64_t savedTSR;
    Exception_Context *excp;
    unsigned int ncnt;

    /* set exception context */
    excp = Exception_Module_state.excContext;

    if (Exception_enablePrint) {
        /* Force MAIN threadtype So we can safely call System_printf */
        DebugP_exceptionLog("A0 =0x%x A1 =0x%x\n", (uintptr_t)excp->A0, (uintptr_t)excp->A1);
        DebugP_exceptionLog("A2 =0x%x A3 =0x%x\n", (uintptr_t)excp->A2, (uintptr_t)excp->A3);
        DebugP_exceptionLog("A4 =0x%x A5 =0x%x\n", (uintptr_t)excp->A4, (uintptr_t)excp->A5);
        DebugP_exceptionLog("A6 =0x%x A7 =0x%x\n", (uintptr_t)excp->A6, (uintptr_t)excp->A7);
        DebugP_exceptionLog("A8 =0x%x A9 =0x%x\n", (uintptr_t)excp->A8, (uintptr_t)excp->A9);
        DebugP_exceptionLog("A10=0x%x A11=0x%x\n", (uintptr_t)excp->A10, (uintptr_t)excp->A11);
        DebugP_exceptionLog("A12=0x%x A13=0x%x\n", (uintptr_t)excp->A12, (uintptr_t)excp->A13);
        DebugP_exceptionLog("A14=0x%x A15=0x%x\n", (uintptr_t)excp->A14, (uintptr_t)excp->A15);
        DebugP_exceptionLog("D0 =0x%x D1 =0x%x\n", (uintptr_t)excp->D0, (uintptr_t)excp->D1);
        DebugP_exceptionLog("D2 =0x%x D3 =0x%x\n", (uintptr_t)excp->D2, (uintptr_t)excp->D3);
        DebugP_exceptionLog("D4 =0x%x D5 =0x%x\n", (uintptr_t)excp->D4, (uintptr_t)excp->D5);
        DebugP_exceptionLog("D6 =0x%x D7 =0x%x\n", (uintptr_t)excp->D6, (uintptr_t)excp->D7);
        DebugP_exceptionLog("D8 =0x%x D9 =0x%x\n", (uintptr_t)excp->D8, (uintptr_t)excp->D9);
        DebugP_exceptionLog("D10=0x%x D11=0x%x\n", (uintptr_t)excp->D10, (uintptr_t)excp->D11);
        DebugP_exceptionLog("D12=0x%x D13=0x%x\n", (uintptr_t)excp->D12, (uintptr_t)excp->D13);
        DebugP_exceptionLog("D14=0x%x D15=0x%x\n", (uintptr_t)excp->D14, (uintptr_t)excp->D15);
        DebugP_exceptionLog("AM0=0x%x AM1=0x%x\n", (uintptr_t)excp->AM0, (uintptr_t)excp->AM1);
        DebugP_exceptionLog("AM2=0x%x AM3=0x%x\n", (uintptr_t)excp->AM2, (uintptr_t)excp->AM3);
        DebugP_exceptionLog("AM4=0x%x AM5=0x%x\n", (uintptr_t)excp->AM4, (uintptr_t)excp->AM5);
        DebugP_exceptionLog("AM6=0x%x AM7=0x%x\n", (uintptr_t)excp->AM6, (uintptr_t)excp->AM7);
        DebugP_exceptionLog("AL0=0x%x AL1=0x%x\n", (uintptr_t)excp->AL0, (uintptr_t)excp->AL1);
        DebugP_exceptionLog("AL2=0x%x AL3=0x%x\n", (uintptr_t)excp->AL2, (uintptr_t)excp->AL3);
        DebugP_exceptionLog("AL4=0x%x AL5=0x%x\n", (uintptr_t)excp->AL4, (uintptr_t)excp->AL5);
        DebugP_exceptionLog("AL6=0x%x AL7=0x%x\n", (uintptr_t)excp->AL6, (uintptr_t)excp->AL7);
        DebugP_exceptionLog("P0=0x%x P1=0x%x\n", (uintptr_t)excp->P0, (uintptr_t)excp->P1);
        DebugP_exceptionLog("P2=0x%x P3=0x%x\n", (uintptr_t)excp->P2, (uintptr_t)excp->P3);
        DebugP_exceptionLog("P4=0x%x P5=0x%x\n", (uintptr_t)excp->P4, (uintptr_t)excp->P5);
        DebugP_exceptionLog("P6=0x%x P7=0x%x\n", (uintptr_t)excp->P6, (uintptr_t)excp->P7);
        DebugP_exceptionLog("FPCR=0x%x FSR=0x%x\n", (uintptr_t)excp->FPCR, (uintptr_t)excp->FSR);
        DebugP_exceptionLog("GFPGFR=0x%x GPLY=0x%x\n",
                      (uintptr_t)excp->GFPGFR, (uintptr_t)excp->GPLY);
        printTwoVectors("VBM0", excp->VBM0, "VBM1", excp->VBM1);
        printTwoVectors("VBM2", excp->VBM2, "VBM3", excp->VBM3);
        printTwoVectors("VBM4", excp->VBM4, "VBM5", excp->VBM5);
        printTwoVectors("VBM6", excp->VBM6, "VBM7", excp->VBM7);
        printTwoVectors("VBL0", excp->VBL0, "VBL1", excp->VBL1);
        printTwoVectors("VBL2", excp->VBL2, "VBL3", excp->VBL3);
        printTwoVectors("VBL4", excp->VBL4, "VBL5", excp->VBL5);
        printTwoVectors("VBL6", excp->VBL6, "VBL7", excp->VBL7);
        printTwoVectors("VB0", excp->VB0, "VB1", excp->VB1);
        printTwoVectors("VB2", excp->VB2, "VB3", excp->VB3);
        printTwoVectors("VB4", excp->VB4, "VB5", excp->VB5);
        printTwoVectors("VB6", excp->VB6, "VB7", excp->VB7);
        printTwoVectors("VB8", excp->VB8, "VB9", excp->VB9);
        printTwoVectors("VB10", excp->VB10, "VB11", excp->VB11);
        printTwoVectors("VB12", excp->VB12, "VB13", excp->VB13);
        printTwoVectors("VB14", excp->VB14, "VB15", excp->VB15);
        printTwoVectors("CUCR0", excp->CUCR0, "CUCR1", excp->CUCR1);
        printTwoVectors("CUCR2", excp->CUCR2, "CUCR3", excp->CUCR3);
        printTwoVectors("SE0_0", excp->SE0_0, "SE0_1", excp->SE0_1);
        printTwoVectors("SE0_2", excp->SE0_2, "SE0_3", excp->SE0_3);
        printTwoVectors("SE1_0", excp->SE1_0, "SE1_1", excp->SE1_1);
        printTwoVectors("SE1_2", excp->SE1_2, "SE1_3", excp->SE1_3);
        printTwoVectors("SA0CR", excp->SA0CR, "SA1CR", excp->SA1CR);
        printTwoVectors("SA2CR", excp->SA2CR, "SA3CR", excp->SA3CR);
        printTwoVectors("SA0CNTR0", excp->SA0CNTR0, "SA1CNTR0", excp->SA1CNTR0);
        printTwoVectors("SA2CNTR0", excp->SA2CNTR0, "SA3CNTR0", excp->SA3CNTR0);

        DebugP_log0("\n");
    }

    ncnt = (__ECSP_S & 0xe000) >> 13;
    if (ncnt) {
        contextStack = (uint64_t *)(((uint64_t)__ECSP_S) + ((ncnt - 1) * 0x2000));
    }
    else  {
        contextStack = (uint64_t *)__TCSP;
    }

    nrp = contextStack[0];
    savedTSR = contextStack[1];

    Exception_Module_state.nrp = nrp;
    Exception_Module_state.ntsr = savedTSR;

    /* print general exception info */
    if (Exception_enablePrint) {
        DebugP_log1("Exception at 0x%016lx\n", (int)Exception_Module_state.nrp);
        DebugP_log1("TSR at time of exception: 0x%016lx\n",
                      (int)Exception_Module_state.ntsr);
    }

    if (*Exception_exceptionHook != NULL) {
        (*Exception_exceptionHook)();
    }

    /* process all possible causes of exception */
    if (vectorType == 0) {
        /* internal exception */
        DebugP_log0("Internal exception:\n");
        Exception_internalHandler();
    }
    if (vectorType == 1) {
        /* page fault */
        DebugP_log0("Page fault:\n");
        Exception_internalHandler();
    }

    if (abortFlag) {
        if (Exception_enablePrint) {
            DebugP_log2("Exception_E_exceptionMax", nrp, (uintptr_t)excp->D15);
        }
        else {
            DebugP_log2("Exception_E_exceptionMin", nrp, (uintptr_t)excp->D15);
        }
    }
}

/*
 *  ======== Exception_internalHandler ========
 *  This function is called only if an internal exception has occurred.
 *  It checks the EFR register to determine what type of exceptions occurred.
 */
void Exception_internalHandler(void)
{
    unsigned int ierr;
    unsigned int iesr19_16;
    unsigned int iesr15_0;

    /* record IERR in Exc_module field */
    Exception_Module_state.ierr = __IERR;
    Exception_Module_state.iear = __IEAR;
    Exception_Module_state.iesr = __IESR;

    if (Exception_enablePrint) {
        DebugP_log3("  IERR=0x%016lx\n"
                      "  IEAR=0x%016lx\n"
                      "  IESR=0x%016lx\n",
                      Exception_Module_state.ierr,
                      Exception_Module_state.iear,
                      Exception_Module_state.iesr);

        iesr19_16 = (Exception_Module_state.iesr & 0xf0000) >> 16;
        iesr15_0 = Exception_Module_state.iesr & 0xffff;
        ierr = Exception_Module_state.ierr;

        if (ierr & Exception_IERRPFX) {
            DebugP_log0("  Page fault exception:\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log1("    uTLB Fault, cpu_pmc_rstatus[10:0]=0x%x\n",
                              iesr15_0 & 0x7ff);
                break;
              case 1:
                DebugP_log0("    .D1 or .D2 uTLB lookup fault, Non-speculative load\n");
                DebugP_log1("    utlb_rstatus=0x%x\n", iesr15_0);
                break;
              case 2:
                DebugP_log0("    .D1 or .D2 uTLB lookup fault, Speculative load\n");
                DebugP_log1("    utlb_rstatus=0x%x\n", iesr15_0);
                break;
              case 3:
                DebugP_log1("    uTLB Fault, cpu_se_N_rstatus[10:0]=0x%x\n",
                              iesr15_0 & 0x7ff);
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERRIFX) {
            DebugP_log1("  Instruction fetch exception, cpu_pmc_rstatus[2:0]=0x%x\n",
                          iesr15_0 & 0x7);
        }
        if (ierr & Exception_IERRFPX) {
            DebugP_log0("  Fetch packet exception\n");
        }
        if (ierr & Exception_IERREPX) {
            DebugP_log0("  Execute packet exception\n");
        }
        if (ierr & Exception_IERROPX) {
            DebugP_log0("  Illegal opcode exception\n");
        }
        if (ierr & Exception_IERRRCX) {
            DebugP_log0("  Resource conflict exception\n");
        }
        if (ierr & Exception_IERRRAX) {
            DebugP_log0("  Resource access exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log0("    *SE access when stream is not open\n");
                break;
              case 1:
                DebugP_log0("    SESAVE or SERSTR while stream is open\n");
                break;
              case 2:
                DebugP_log0("    LUT ,HIST when the access bit = 0\n");
                DebugP_log1("    Histogram Base CR #: %d\n", iesr15_0);
                break;
              case 3:
                DebugP_log0("    ECR Non-zero Rstatus except for rstatus == 2 (permission violation)\n");
                DebugP_log1("    cpu_to_ecr_status=0x%x\n", iesr15_0);
                break;
              case 4:
                DebugP_log0("    DSWBP when SWBP en == 0\n");
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERRPRX) {
            DebugP_log0("  Privilege exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log0("    Internal MVC permission violation\n");
                break;
              case 1:
                DebugP_log0("    External MVC permission violation\n");
                DebugP_log1("    cpu_to_ecr_status=0x%x\n", iesr15_0);
                break;
              case 2:
                DebugP_log0("    Invalid Permissions for given opcode\n");
                break;
              case 3:
                DebugP_log0("    Illegal RETE/RETS execution\n");
                break;
              case 4:
                DebugP_log0("    ECSP addressing mode attempted as a user\n");
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERRLBX) {
            DebugP_log0("  Loop buffer exception\n");
        }
        if (ierr & Exception_IERRMSX) {
            DebugP_log0("  Missed stall exception\n");
        }
        if (ierr & Exception_IERRDFX) {
            DebugP_log0("  Data fetch exception\n");
            DebugP_log1("  cpu_dmc_X_rstatus[2:0]=0x%x\n", iesr15_0 & 0x7);
        }
        if (ierr & Exception_IERRSEX) {
            DebugP_log0("  Streaming engine exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log1("    L2 Return Error, cpu_se_N_rstatus[2:0]=0x%x\n",
                              iesr15_0 & 0x7);
                break;
              case 1:
                DebugP_log1("    SE Internal Error, cpu_se_N_rstatus[2:0]=0x%x\n",
                              iesr15_0 & 0x7);
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERREXX) {
            DebugP_log0("  Execution exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log0("    Executing PROT-mode only instructions in UNPROT mode\n");
                break;
              case 1:
                DebugP_log0("    Division by zero\n");
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERRADX) {
            DebugP_log0("  Address exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log0("    Out-of-range address on DMC memory address\n");
                break;
              case 1:
                DebugP_log0("    Out-of-range address on CR memory address\n");
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
        if (ierr & Exception_IERRMMX) {
            DebugP_log0("  MMA exception\n");
            switch (iesr19_16) {
              case 0:
                DebugP_log0("    MMA instruction when MMA isn't present\n");
                break;
              case 1:
                DebugP_log0("    MMA instruction when MMA isn't active\n");
                break;
              case 2:
                DebugP_log0("    MMA status error\n");
	        DebugP_log1("    mma_rstatus=0x%x\n", iesr15_0);
                break;
              default:
                DebugP_log1("    unknown sub-type, iesr[19:16]=0x%x\n",
                              iesr19_16);
                break;
            }
        }
    }

    if (*Exception_internalHook != NULL) {
        (*Exception_internalHook)();
    }

    /* clear internal exceptions to allow them to be recognized again */
    __IERR = 0;
}

/*
 *  ======== Exception_getLastStatus ========
 *  This function returns the last exception status through the status ptr.
 */
void Exception_getLastStatus(Exception_Status *status)
{
    status->nrp = Exception_Module_state.nrp;
    status->ntsr = Exception_Module_state.ntsr;
    status->ierr = Exception_Module_state.ierr;
    status->iear = Exception_Module_state.iear;
    status->iesr = Exception_Module_state.iesr;
    status->excContext = Exception_Module_state.excContext;
}

/*
 *  ======== Exception_clearLastStatus ========
 *  This function clears the last exception status.
 */
void Exception_clearLastStatus(void)
{
    Exception_Module_state.nrp = 0;
    Exception_Module_state.ntsr = 0;
    Exception_Module_state.ierr = 0;
    Exception_Module_state.iear = 0;
    Exception_Module_state.iesr = 0;
    Exception_Module_state.excContext = 0;
}

/*
 *  ======== Exception_setReturnPtr ========
 *  This function sets the new return pointer and returns the old pointer.
 */
Exception_FuncPtr Exception_setReturnPtr(Exception_FuncPtr ptr)
{
    Exception_FuncPtr curPtr;

    curPtr = Exception_Module_state.returnHook;
    Exception_Module_state.returnHook = ptr;

    return curPtr;
}


/* Module_state */
#ifdef __ti__
struct Exception_Module_State Exception_Module_state __attribute__ ((section(".data:Exception_Module_state")));
#elif !(defined(__MACH__) && defined(__APPLE__))
struct Exception_Module_State Exception_Module_state __attribute__ ((section(".data:Exception_Module_state")));
#endif
struct Exception_Module_State Exception_Module_state = {
    (uint64_t)0x0U,  /* nrp */
    (uint64_t)0x0U,  /* ntsr */
    (uint64_t)0x0U,  /* ierr */
    (uint64_t)0x0U,  /* iear */
    (uint64_t)0x0U,  /* iesr */
    ((void(*)(void))NULL),  /* returnHook */
    ((Exception_Context*)NULL),  /* excContext */
    ((char*)NULL),  /* excPtr */
    {0},  /* contextBuf */
};


