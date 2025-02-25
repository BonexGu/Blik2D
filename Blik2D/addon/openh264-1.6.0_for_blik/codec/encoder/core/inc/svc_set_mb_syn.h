/*!
 * \copy
 *     Copyright (c)  2009-2013, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * \file    svc_set_mb_syn.h
 *
 * \brief   Seting all syntax elements of mb and encoding residual with cavlc and cabac
 *
 * \date    2009.8.12 Created
 *
 *************************************************************************************
 */

#ifndef SVC_SET_MB_SYN_H_
#define SVC_SET_MB_SYN_H_

#include BLIK_OPENH264_U_typedefs_h //original-code:"typedefs.h"
#include BLIK_OPENH264_U_wels_common_basis_h //original-code:"wels_common_basis.h"
#include BLIK_OPENH264_U_encoder_context_h //original-code:"encoder_context.h"
#include BLIK_OPENH264_U_md_h //original-code:"md.h"
#include BLIK_OPENH264_U_slice_h //original-code:"slice.h"
#include BLIK_OPENH264_U_set_mb_syn_cavlc_h //original-code:"set_mb_syn_cavlc.h"
#include BLIK_OPENH264_U_set_mb_syn_cabac_h //original-code:"set_mb_syn_cabac.h"

namespace WelsEnc {



int32_t WelsWriteMbResidual (SWelsFuncPtrList* pFuncList, SMbCache* sMbCacheInfo, SMB* pCurMb, SBitStringAux* pBs);

void WelsSpatialWriteSubMbPred (sWelsEncCtx* pEncCtx, SSlice* pSlice, SMB* pCurMb);

void WelsSpatialWriteMbPred (sWelsEncCtx* pEncCtx, SSlice* pSlice, SMB* pCurMb);
void WelsInitSliceCabac(sWelsEncCtx* pEncCtx,SSlice* pSlice);
void WelsCabacInit(void *pCtx);
void WelsWriteSliceEndSyn(SSlice *pSlice,bool bEntropyCodingModeFlag);
//for Base Layer CAVLC writing
int32_t WelsSpatialWriteMbSyn (sWelsEncCtx* Ctx, SSlice* pSlice, SMB* pCurMb);
int32_t WelsSpatialWriteMbSynCabac (sWelsEncCtx* pCtx, SSlice* pSlice, SMB* pCurMb);
int32_t GetBsPosCavlc(SSlice *pSlice);
int32_t GetBsPosCabac(SSlice *pSlice);
}
#endif
