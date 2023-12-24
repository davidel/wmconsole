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




int WmcCmd_chmod(WmcCtx *pCtx, char **ppszArgsA) {
	DWORD dwAttrMask = 0, dwAttrMaskN = 0, dwAttr;
	char const *pszAttr;
	wchar_t *pszPathW = NULL;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (ppszArgsA[1][0] != '-' && ppszArgsA[1][0] != '+') {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Wrong attribute set {'+', '-'}[hsw]+\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	for (pszAttr = ppszArgsA[1] + 1; *pszAttr != '\0'; pszAttr++) {
		switch (*pszAttr) {
		case 's':
			dwAttrMask |= FILE_ATTRIBUTE_SYSTEM;
			break;
		case 'h':
			dwAttrMask |= FILE_ATTRIBUTE_HIDDEN;
			break;
		case 'w':
			dwAttrMaskN |= FILE_ATTRIBUTE_READONLY;
			break;
		}
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[2])) == NULL ||
	    (dwAttr = GetFileAttributes(pszPathW)) == (DWORD) -1) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[2]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (ppszArgsA[1][0] == '-') {
		dwAttr &= ~dwAttrMask;
		dwAttr |= dwAttrMaskN;
	} else {
		dwAttr |= dwAttrMask;
		dwAttr &= ~dwAttrMaskN;
	}
	if (!SetFileAttributes(pszPathW, dwAttr)) {
		free(pszPathW);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to set attributes: '%s'\n",
					 ppszArgsA[2]) < 0)
			return -1;
		return WMC_ERR_MKDIR;
	}
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

