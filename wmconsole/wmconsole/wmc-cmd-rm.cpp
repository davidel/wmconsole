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




int WmcCmd_rm(WmcCtx *pCtx, char **ppszArgsA) {
	int i;
	wchar_t *pszPathW;

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	for (i = 1; ppszArgsA[i] != NULL; i++) {
		if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[i])) == NULL) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
					  ppszArgsA[i]) < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_INVALIDARG;
		}
		SetFileAttributes(pszPathW,
				  GetFileAttributes(pszPathW) & ~FILE_ATTRIBUTE_READONLY);
		if (!DeleteFile(pszPathW)) {
			free(pszPathW);
			if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
						 "Unable to remove file: '%s'\n",
						 ppszArgsA[i]) < 0)
				return -1;
			return WMC_ERR_RMFILE;
		}
		free(pszPathW);
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

