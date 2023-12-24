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

