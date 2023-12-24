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



#define WMC_DEVLIST_BUFSIZE (16 * 1024)





int WmcCmd_lsdev(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	DWORD dwSize;
	wchar_t *pszDevBuf, *pszDevW;
	char *pszDev;

	dwSize = WMC_DEVLIST_BUFSIZE;
	if ((pszDevBuf = (wchar_t *) malloc(dwSize)) == NULL) {
		WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Out of memory\n");
		WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0);
		return -1;
	}
	if (EnumDevices(pszDevBuf, &dwSize) != ERROR_SUCCESS) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Cannot enumerate devices\n") < 0)
			return -1;
		return WMC_ERR_DEVENUM;
	}
	for (pszDevW = pszDevBuf; *pszDevW; pszDevW += wcslen(pszDevW) + 1) {
		pszDev = WmcWide2Byte(pszDevW, -1, 0);
		iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
				       "%s\n", pszDev);
		free(pszDev);
		if (iError < 0) {
			free(pszDevBuf);
			return -1;
		}
	}
	free(pszDevBuf);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

