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

