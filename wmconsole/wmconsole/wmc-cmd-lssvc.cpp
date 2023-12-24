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



#define WMC_SVCLIST_BUFSIZE (32 * 1024)





int WmcCmd_lssvc(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	DWORD i, dwEnts, dwSize;
	ServiceEnumInfo *pSEI;
	char *pszPrefix, *pszDllName;

	dwSize = WMC_SVCLIST_BUFSIZE;
	if ((pSEI = (ServiceEnumInfo *) malloc(dwSize)) == NULL) {
		WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Out of memory\n");
		WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0);
		return -1;
	}
	dwEnts = 0;
	if (!EnumServices((PBYTE) pSEI, &dwEnts, &dwSize)) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Cannot enumerate services\n") < 0)
			return -1;
		return WMC_ERR_SVCENUM;
	}
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "PFIX    STATUS   FILE\n") < 0) {
		free(pSEI);
		return -1;
	}
	for (i = 0; i < dwEnts; i++) {
		pszPrefix = WmcWide2Byte(pSEI[i].szPrefix, -1, 0);
		pszDllName = WmcWide2Byte(pSEI[i].szDllName, -1, 0);
		iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
				       "%-8s%-9ld%s\n",
				       pszPrefix, (long) pSEI[i].dwServiceState,
				       pszDllName);
		free(pszDllName);
		free(pszPrefix);
		if (iError < 0) {
			free(pSEI);
			return -1;
		}
	}
	free(pSEI);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

