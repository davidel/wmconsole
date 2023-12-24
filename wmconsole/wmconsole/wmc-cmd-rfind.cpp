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




static int WmcCmdRfindEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf);




static int WmcCmdRfindEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf) {
	WmcCtx *pCtx = (WmcCtx *) pPrivate;
	int iError;
	char *pszLsStr;

	pszLsStr = WmcRegGetLsStringW(pszPathW, pKInf);
	iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszLsStr);
	free(pszLsStr);

	return iError;
}

int WmcCmd_rfind(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	wchar_t *pszPathW, *pszMatchW;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
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
	if ((pszMatchW = WmcByte2Wide(ppszArgsA[2], -1, 0)) == NULL) {
		free(pszPathW);
		return -1;
	}
	iError = WmcRegWalkTreeW(pszPathW, pszMatchW, 1, WmcCmdRfindEnumCb, pCtx);
	free(pszMatchW);
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return iError;
}

