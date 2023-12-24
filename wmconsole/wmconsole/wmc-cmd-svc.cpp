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




int WmcCmd_svc(WmcCtx *pCtx, char **ppszArgsA) {
	DWORD dwIoctl;
	wchar_t *pszDevW;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (strcmp(ppszArgsA[1], "-S") == 0)
		dwIoctl = IOCTL_SERVICE_START;
	else if (strcmp(ppszArgsA[1], "-T") == 0)
		dwIoctl = IOCTL_SERVICE_STOP;
	else {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid action: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((pszDevW = WmcByte2Wide(ppszArgsA[2], -1, 0)) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid argoument: '%s'\n",
				  ppszArgsA[2]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcSimpleDevIoctl(pszDevW, dwIoctl) < 0) {
		free(pszDevW);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to control service: '%s'\n",
					 ppszArgsA[2]) < 0)
			return -1;
		return WMC_ERR_SVCIOCTL;
	}
	free(pszDevW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

