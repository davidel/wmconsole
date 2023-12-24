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




int WmcCmd_put(WmcCtx *pCtx, char **ppszArgsA) {
	int iChNbr, iError;
	HANDLE hFile;
	unsigned long ulFSize, ulRcvSize;
	long lPktSize;
	void *pPktData;
	wchar_t *pszPathW;

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
	if (_stricmp(ppszArgsA[0], "putf") == 0 &&
	    WmcMkAllDirsW(pszPathW, 0) < 0) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot create: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((hFile = CreateFile(pszPathW, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				NULL)) == INVALID_HANDLE_VALUE) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot create: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0 ||
	    WmcPktRecv(pCtx->pCh, &iChNbr, &pPktData, &lPktSize) < 0) {
		CloseHandle(hFile);
		DeleteFile(pszPathW);
		free(pszPathW);
		return -1;
	}
	if (lPktSize != 4) {
		free(pPktData);
		CloseHandle(hFile);
		DeleteFile(pszPathW);
		free(pszPathW);
		return -1;
	}
	WMCM_GET_LE32(ulFSize, pPktData);
	free(pPktData);
	for (ulRcvSize = 0, iError = 0;;) {
		DWORD dwWrite = 0;

		if (WmcPktRecv(pCtx->pCh, &iChNbr, &pPktData, &lPktSize) < 0) {
			CloseHandle(hFile);
			DeleteFile(pszPathW);
			free(pszPathW);
			return -1;
		}
		if (lPktSize == 0) {
			free(pPktData);
			break;
		}
		if (iError == 0 &&
		    (!WriteFile(hFile, pPktData, (DWORD) lPktSize, &dwWrite, NULL) ||
		     dwWrite != (DWORD) lPktSize))
			iError = -1;
		free(pPktData);
		ulRcvSize += (unsigned long) dwWrite;
	}
	CloseHandle(hFile);
	if (iError < 0) {
		DeleteFile(pszPathW);
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot write: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_FILEWRITE;
	}
	if (ulRcvSize != ulFSize) {
		DeleteFile(pszPathW);
		free(pszPathW);
		return -1;
	}
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

