//    Copyright 2023 Davide Libenzi
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// 


#include "wmc-includes.h"





int WmcCmd_rcat(WmcCtx *pCtx, char **ppszArgsA) {
	char *pszVStr;
	wchar_t *pszPathW, *pszVStrW;
	WmcRValData RVal;

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszRegCWD, ppszArgsA[1])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcRegGetValueW(pszPathW, &RVal) < 0) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Registry value path not found: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_NOTFOUND;
	}
	free(pszPathW);
	pszVStrW = WmcRegVal2String(&RVal);
	WmcRegFreeValue(&RVal);
	if (pszVStrW == NULL)
		return -1;
	pszVStr = WmcWide2Byte(pszVStrW, -1, 0);
	free(pszVStrW);
	if (pszVStr == NULL)
		return -1;
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszVStr) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;
	free(pszVStr);

	return 0;
}

