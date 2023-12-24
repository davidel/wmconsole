/*
 *  WmConsole by Davide Libenzi (Windows Mobile console server)
 *  Copyright (C) 2006  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

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

