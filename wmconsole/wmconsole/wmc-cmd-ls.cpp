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




int WmcCmd_ls(WmcCtx *pCtx, char **ppszArgsA) {
	HANDLE hFind;
	char *pszLsPath, *pszTmp;
	wchar_t *pszLsPathW;
	WIN32_FIND_DATA WFD;

	if (ppszArgsA[1] != NULL) {
		if ((pszLsPath = WmcGetPath(pCtx->pszCWD, ppszArgsA[1])) == NULL) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
					  ppszArgsA[1]) < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_INVALIDARG;
		}
		if (!WmcWildPath(pszLsPath)) {
			if (WmcGetPathInfo(pszLsPath, &WFD) < 0) {
				free(pszLsPath);
				if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Path not found: '%s'\n",
						  ppszArgsA[1]) < 0 ||
				    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
					return -1;
				return WMC_ERR_NOTFOUND;
			}
			if (WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				WMCM_DROPLAST(pszLsPath, '\\');
				pszTmp = WmcSPrintf("%s\\*", pszLsPath);
				free(pszLsPath);
				pszLsPath = pszTmp;
			}
		}
	} else
		pszLsPath = WmcSPrintf("%s*", pCtx->pszCWD);
	if (pszLsPath == NULL)
		return -1;
	pszLsPathW = WmcByte2Wide(pszLsPath, -1, 0);
	free(pszLsPath);
	if (pszLsPathW == NULL)
		return -1;
	hFind = FindFirstFile(pszLsPathW, &WFD);
	free(pszLsPathW);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((pszTmp = WmcGetLsString(NULL, &WFD)) != NULL &&
			    WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszTmp) < 0) {
				free(pszTmp);
				FindClose(hFind);
				return -1;
			}
			free(pszTmp);
		} while (FindNextFile(hFind, &WFD));
		FindClose(hFind);
	} else {
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Path not found\n") < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_NOTFOUND;
		}
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

