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




int WmcCmd_mkdir(WmcCtx *pCtx, char **ppszArgsA) {
	int iArgN, iError, iMkAll = 0;
	wchar_t *pszPathW;

	for (iArgN = 1; ppszArgsA[iArgN] != NULL; iArgN++) {
		if (strcmp(ppszArgsA[iArgN], "-p") == 0)
			iMkAll++;
		else
			break;
	}
	if (ppszArgsA[iArgN] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[iArgN])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[iArgN]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (iMkAll)
		iError = WmcMkAllDirsW(pszPathW, 1);
	else
		iError = CreateDirectory(pszPathW, NULL) ? 0: -1;
	if (iError < 0) {
		free(pszPathW);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to create directory: '%s'\n",
					 ppszArgsA[iArgN]) < 0)
			return -1;
		return WMC_ERR_MKDIR;
	}
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

