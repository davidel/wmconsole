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




int WmcCmd_get(WmcCtx *pCtx, char **ppszArgsA) {
	HANDLE hFile;
	unsigned long ulFSize, ulSndSize;
	wchar_t *pszPathW;
	char TxBuf[2048];

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[1])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	hFile = CreateFile(pszPathW, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0,
			   NULL);
	free(pszPathW);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot open: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	ulFSize = GetFileSize(hFile, NULL);
	WMCM_PUT_LE32(ulFSize, TxBuf);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, TxBuf, 4) < 0) {
		CloseHandle(hFile);
		return -1;
	}
	for (ulSndSize = 0; ulSndSize < ulFSize;) {
		DWORD dwRead = 0, cwCRead;

		cwCRead = (ulSndSize - ulFSize > sizeof(TxBuf)) ?
			sizeof(TxBuf): (DWORD) (ulSndSize - ulFSize);
		if (!ReadFile(hFile, TxBuf, cwCRead, &dwRead, NULL))
			break;
		if (dwRead != 0 &&
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, TxBuf, (long) dwRead) < 0) {
			CloseHandle(hFile);
			return -1;
		}
		ulSndSize += (unsigned long) dwRead;
	}
	CloseHandle(hFile);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

