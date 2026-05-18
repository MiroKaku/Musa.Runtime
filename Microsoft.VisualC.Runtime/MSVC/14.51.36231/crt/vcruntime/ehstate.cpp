/***
* ehstate.cpp
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*Purpose:
*       Contains state management code for all platforms.
*
*/

#include <vcruntime_internal.h>
#include <eh.h>
#include <ehassert.h>
#include <ehdata.h>
#include <ehdata4.h>
#include <ehhooks.h>
#include <trnsctrl.h>
#include "ehhelpers.h"
#include <windows.h>

#if _EH_RELATIVE_FUNCINFO
#if defined(_M_ARM64) || defined(_CHPE_X86_ARM64_EH_) || defined(_M_ARM64EC)
static uintptr_t adjustIp(DispatcherContext *pDC, uintptr_t Ip)
{
#if defined(_M_ARM64EC)
    if(RtlIsEcCode(Ip)) {
        PDISPATCHER_CONTEXT_ARM64EC pECDC;
        pECDC = (PDISPATCHER_CONTEXT_ARM64EC) pDC;

        if(pECDC->ControlPcIsUnwound != FALSE) {
            Ip -= 4;
        }
    }
#else

   //
   // If this context came from an unwind to a call, then the ControlPc points
   // to a return address, which could put us at the start of a neighboring
   // scope. To correct for this, back the PC up by the minimum instruction
   // size to ensure we are in the same scope as the original call opcode.
   //
   if (pDC->ControlPcIsUnwound) {
        Ip -= 4;
    }

#endif // _M_ARM64EC
    return Ip;
}

#else
static uintptr_t adjustIp(DispatcherContext* /*pDC*/, uintptr_t Ip)
{
    return Ip;
}
#endif // defined(_M_ARM64) || defined(_CHPE_X86_ARM64_EH_) || defined(_M_ARM64EC)

__ehstate_t RENAME_EH_EXTERN(__FrameHandler4)::StateFromIp(
    FuncInfo            *pFuncInfo,
    DispatcherContext   *pDC,
    uintptr_t           Ip
)
{
    unsigned int        index;          //  loop control variable
    unsigned int        nIPMapEntry;    //  # of IpMapEntry; must be > 0

    Ip = adjustIp(pDC, Ip);

    if (pFuncInfo->dispIPtoStateMap == 0)
    {
        return EH_EMPTY_STATE;
    }

    PBYTE buffer = (PBYTE)__RVAtoRealOffset(pDC, pFuncInfo->dispIPtoStateMap);

    nIPMapEntry = FH4::ReadUnsigned(&buffer);

    __ehstate_t prevState = EH_EMPTY_STATE;
    unsigned int funcRelIP = 0;
    for (index = 0; index < nIPMapEntry; index++) {
        // States are delta-encoded relative to start of the function
        funcRelIP += FH4::ReadUnsigned(&buffer);
        if (Ip < __FuncRelToRealOffset(pDC, funcRelIP)) {
            break;
        }
        // States are encoded +1 so as to not encode a negative
        prevState = FH4::ReadUnsigned(&buffer) - 1;
    }

    if (index == 0) {
        // We are at the first entry, could be an error
        return EH_EMPTY_STATE;
    }

    // We over-shot one iteration; return state from the previous slot
    return prevState;
}

__ehstate_t RENAME_EH_EXTERN(__FrameHandler3)::StateFromIp(
    FuncInfo            *pFuncInfo,
    DispatcherContext   *pDC,
    uintptr_t           Ip
)
{

    unsigned int        index;          //  loop control variable
    unsigned int        nIPMapEntry;    //  # of IpMapEntry; must be > 0

    Ip = adjustIp(pDC, Ip);

    _VCRT_VERIFY(pFuncInfo);
    nIPMapEntry = FUNC_NIPMAPENT(*pFuncInfo);

    _VCRT_VERIFY(FUNC_IPMAP(*pFuncInfo, pDC->ImageBase));

    for (index = 0; index < nIPMapEntry; index++) {
        IptoStateMapEntry *pIPtoStateMap = FUNC_PIPTOSTATE(*pFuncInfo, index, pDC->ImageBase);
        if (Ip < (uintptr_t)__RVAtoRealOffset(pDC, pIPtoStateMap->Ip)) {
            break;
        }
    }

    if (index == 0) {
        // We are at the first entry, could be an error

        return EH_EMPTY_STATE;
    }

    // We over-shot one iteration; return state from the previous slot

    return FUNC_IPTOSTATE(*pFuncInfo, index - 1, pDC->ImageBase).State;

}

__ehstate_t RENAME_EH_EXTERN(__FrameHandler3)::StateFromControlPc(
    FuncInfo           *pFuncInfo,
    DispatcherContext  *pDC
)
{
    uintptr_t Ip = pDC->ControlPc;

    return StateFromIp(pFuncInfo, pDC, Ip);
}

__ehstate_t RENAME_EH_EXTERN(__FrameHandler4)::StateFromControlPc(
    FuncInfo           *pFuncInfo,
    DispatcherContext  *pDC
)
{
    uintptr_t Ip = pDC->ControlPc;

    return StateFromIp(pFuncInfo, pDC, Ip);
}

void RENAME_EH_EXTERN(__FrameHandler3)::SetUnwindTryBlock(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo,
    int                 curState
)
{
    EHRegistrationNode EstablisherFramePointers;
    EstablisherFramePointers = *RENAME_EH_EXTERN(__FrameHandler3)::GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
    int fixed_adj = FUNC_DISPUNWINDHELP(*pFuncInfo), sve_adj = 0;
#if defined(_M_ARM64) && !defined(_M_ARM64EC)
    __updateAdjForSve(pFuncInfo, pDC->ContextRecord, fixed_adj, sve_adj);
#endif
    if (curState > UNWINDTRYBLOCK(EstablisherFramePointers, fixed_adj, sve_adj)) {
        UNWINDTRYBLOCK(EstablisherFramePointers, fixed_adj, sve_adj) = (int)curState;
    }
}

__ehstate_t RENAME_EH_EXTERN(__FrameHandler3)::GetUnwindTryBlock(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
)
{
    EHRegistrationNode EstablisherFramePointers;
    EstablisherFramePointers = *RENAME_EH_EXTERN(__FrameHandler3)::GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
    int fixed_adj = FUNC_DISPUNWINDHELP(*pFuncInfo), sve_adj = 0;
#if defined(_M_ARM64) && !defined(_M_ARM64EC)
    __updateAdjForSve(pFuncInfo, pDC->ContextRecord, fixed_adj, sve_adj);
#endif
    return UNWINDTRYBLOCK(EstablisherFramePointers, fixed_adj, sve_adj);
}

//     +----------+ <--- established frame
//     | callee   |
//     |----------|
//     |....      |
//     |----------|
//     | sve area |
//     |----------|
//     | locals   |
//     |----------|
//     | unwinder |
//     | helper   |
//     +----------+
//
// compiler generates 'fixed_adj' to tell vcruntime the size of fixed area, and
// generates 'sve_adj' for calculating the size of SVE area. We need to use both
// to address 'unwinder helper'
__ehstate_t RENAME_EH_EXTERN(__FrameHandler3)::GetCurrentState(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
)
{
    int fixed_adj = FUNC_DISPUNWINDHELP(*pFuncInfo), sve_adj = 0;
#if defined(_M_ARM64) && !defined(_M_ARM64EC)
    __updateAdjForSve(pFuncInfo, pDC->ContextRecord, fixed_adj, sve_adj);
#endif
    if (UNWINDSTATE(*pRN, fixed_adj, sve_adj) == -2) {
        return RENAME_EH_EXTERN(__FrameHandler3)::StateFromControlPc(pFuncInfo, pDC);
    }
    else {
        return UNWINDSTATE(*pRN, fixed_adj, sve_adj);
    }
}

void RENAME_EH_EXTERN(__FrameHandler3)::SetState(
    EHRegistrationNode  *pRN,
    [[maybe_unused]] DispatcherContext *pDC,
    FuncInfo            *pFuncInfo,
    __ehstate_t          newState
)
{
    int fixed_adj = FUNC_DISPUNWINDHELP(*pFuncInfo), sve_adj = 0;
#if defined(_M_ARM64) && !defined(_M_ARM64EC)
    __updateAdjForSve(pFuncInfo, pDC->ContextRecord, fixed_adj, sve_adj);
#endif
    UNWINDSTATE(*pRN, fixed_adj, sve_adj) = newState;
}
#else
__ehstate_t RENAME_EH_EXTERN(__FrameHandler3)::GetCurrentState(
    EHRegistrationNode  *pRN,
    DispatcherContext*  /*pDC*/,
    FuncInfo            *pFuncInfo
)
{
    // In the initial implementation, the state is simply stored in the registration node.
    // Added support for byte states when max state <= 128. Note that max state is 1+real max state
    if (pFuncInfo->maxState <= 128)
    {
        return (__ehstate_t)(signed char)((pRN)->state & 0xff);
    }
    else {
        return (pRN)->state;
    }
}

void RENAME_EH_EXTERN(__FrameHandler3)::SetState(
    EHRegistrationNode  *pRN,
    DispatcherContext  * /*pDC*/,
    FuncInfo*           /*pFuncInfo*/,
    __ehstate_t         newState
)
{
    pRN->state = newState;
}
#endif
