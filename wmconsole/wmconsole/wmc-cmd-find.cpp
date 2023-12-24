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




typedef struct s_WmcCmdFindCtx {
	WmcCtx *pCtx;
	int iSimple;
} WmcCmdFindCtx;




static int WmcCmdFindWalkCb(void *pPrivate, wchar_t const *pszPathW,
			    WIN32_FIND_DATA const *pWFD);




static int WmcCmdFindWalkCb(void *pPrivate, wchar_t const *pszPathW,
			    WIN32_FIND_DATA const *pWFD) {
	WmcCmdFindCtx *pFCtx = (WmcCmdFindCtx *) pPrivate;
	int iError;
	char *pszPath, *pszLsStr;

	if ((pszPath = WmcWide2Byte(pszPathW, -1, 0)) == NULL)
		return -1;
	if (pFCtx->iSimple)
		pszLsStr = pszPath;
	else {
		if ((pszLsStr = WmcGetLsString(pszPath, pWFD)) == NULL) {
			free(pszPath);
			return -1;
		}
		free(pszPath);
	}
	iError = WmcChanPrintf(pFCtx->pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszLsStr);
	free(pszLsStr);

	return iError;
}

int WmcCmd_find(WmcCtx *pCtx, char **ppszArgsA) {
	int iArgN = 1, iRecurse = 1, iSimple = 0, iError;
	char *pszArg;
	wchar_t *pszPathW, *pszMatchW;
	WmcCmdFindCtx FCtx;

	if (ppszArgsA[iArgN] != NULL && ppszArgsA[iArgN][0] == '-') {
		for (pszArg = ppszArgsA[iArgN] + 1; *pszArg != '\0'; pszArg++) {
			switch (*pszArg) {
			case '1':
				iRecurse = 0;
				break;
			case 's':
				iSimple = 1;
				break;
			}
		}
		iArgN++;
	}
	if (ppszArgsA[iArgN] == NULL || ppszArgsA[iArgN + 1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
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
	iArgN++;
	if ((pszMatchW = WmcByte2Wide(ppszArgsA[iArgN], -1, 0)) == NULL) {
		free(pszPathW);
		return -1;
	}
	WMCM_ZERODATA(FCtx);
	FCtx.pCtx = pCtx;
	FCtx.iSimple = iSimple;
	iError = WmcWalkTreeW(pszPathW, pszMatchW, iRecurse, WmcCmdFindWalkCb, &FCtx);
	free(pszMatchW);
	free(pszPathW);
	if (iError < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

