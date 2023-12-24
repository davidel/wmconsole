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




int WmcCmd_rmkkey(WmcCtx *pCtx, char **ppszArgsA) {
	HKEY hKey, hSubKey;
	LONG lResult;
	DWORD dwDisp;
	wchar_t *pszPathW, *pszKNameW;

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszRegCWD, ppszArgsA[1])) == NULL ||
	    (pszKNameW = WmcRegValuePos(pszPathW)) == NULL) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	*pszKNameW++ = 0;
	if (WmcRegOpenKeyW(&hKey, pszPathW) < 0) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_NOTFOUND;
	}
	lResult = RegCreateKeyEx(hKey, pszKNameW, 0, NULL, REG_OPTION_NON_VOLATILE,
				 0, NULL, &hSubKey, &dwDisp);
	RegCloseKey(hKey);
	free(pszPathW);
	if (lResult != ERROR_SUCCESS) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to create key: '%s'\n",
					 ppszArgsA[1]) < 0)
			return -1;
		return WMC_ERR_REGKEYCREATE;
	}
	RegCloseKey(hSubKey);
	if (dwDisp != REG_CREATED_NEW_KEY) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Key path already exists: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_ALREADYEXIST;
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

