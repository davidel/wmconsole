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




static void WmcCtxGenLoginStr(WmcCtx *pCtx);




int WmcCtxCreate(WmcCtx **ppCtx, WmcSvcConfig *pWcfg) {
	WmcCtx *pCtx;

	if ((pCtx = (WmcCtx *) malloc(sizeof(WmcCtx))) == NULL)
		return -1;
	WMCM_ZERODATA(*pCtx);
	pCtx->pWcfg = pWcfg;
	pCtx->pszCWD = _strdup("\\");
	pCtx->pszRegCWD = _strdup("\\");
	if (WSAStartup(MAKEWORD(2, 2), &pCtx->wsaData) != 0) {
		free(pCtx);
		return -1;
	}
	if ((pCtx->SSkFd = WmcOpenSvrConn(SdpRec, iSdpRecSize, iSdpRecChOff,
					  pWcfg->iChan, &pCtx->ulHRec)) == INVALID_SOCKET) {
		WSACleanup();
		free(pCtx);
		return -1;
	}
	*ppCtx = pCtx;

	return 0;
}

void WmcCtxClose(WmcCtx *pCtx) {

	if (pCtx->pCh != NULL)
		WmcFreeSockChannel(pCtx->pCh, 1);
	closesocket(pCtx->SSkFd);
	WSACleanup();
	free(pCtx->pszRegCWD);
	free(pCtx->pszCWD);
	free(pCtx);
}

static void WmcCtxGenLoginStr(WmcCtx *pCtx) {
	int i;
	SYSTEMTIME STm;

	GetSystemTime(&STm);
	srand((unsigned int) WmcHashGen(&STm, sizeof(STm)));
	for (i = 0; i < 8; i++)
		sprintf(pCtx->szRndStr + 2 * i, "%02x", rand() & 0xff);
}

int WmcCtxSessionOpen(WmcCtx *pCtx) {
	int iNameLen;

	iNameLen = sizeof(pCtx->PeerBTH);
	if ((pCtx->SkFd = accept(pCtx->SSkFd, (sockaddr *) &pCtx->PeerBTH,
				 &iNameLen)) == INVALID_SOCKET)
		return -1;
	if (WmcMkSockChannel(&pCtx->pCh, pCtx->SkFd) < 0) {
		closesocket(pCtx->SkFd);
		return -1;
	}
	WmcCtxGenLoginStr(pCtx);

	return 0;
}

int WmcCtxSessionClose(WmcCtx *pCtx) {

	if (pCtx->pCh == NULL)
		return -1;
	WmcFreeSockChannel(pCtx->pCh, 1);
	pCtx->pCh = NULL;

	return 0;
}

