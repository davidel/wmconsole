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





int WmcCmd_rset(WmcCtx *pCtx, char **ppszArgsA) {
	HKEY hKey;
	LONG lResult;
	DWORD dwType, dwError;
	long lSize;
	void *pData;
	wchar_t *pszPathW, *pszKNameW;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL || ppszArgsA[3] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (_stricmp(ppszArgsA[2], "BI") == 0)
		dwType = REG_BINARY;
	else if (_stricmp(ppszArgsA[2], "SZ") == 0)
		dwType = REG_SZ;
	else if (_stricmp(ppszArgsA[2], "DW") == 0)
		dwType = REG_DWORD;
	else {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid data type: '%s'\n",
				  ppszArgsA[2]) < 0 ||
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
	if ((pData = WmcRegParseData(dwType, ppszArgsA[3], &lSize)) == NULL) {
		RegCloseKey(hKey);
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid data format: '%s'\n",
				  ppszArgsA[3]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	lResult = RegSetValueEx(hKey, pszKNameW, 0, dwType, (LPBYTE) pData, (DWORD) lSize);
	dwError = GetLastError();
	RegCloseKey(hKey);
	free(pData);
	free(pszPathW);
	if (lResult != ERROR_SUCCESS) {
		if (WmcChanOSErrorReportE(pCtx->pCh, WMC_CHAN_CTRL, dwError,
					  "Unable to set value: '%s' = '%s'\n",
					  ppszArgsA[1], ppszArgsA[3]) < 0)
			return -1;
		return WMC_ERR_REGVALUESET;
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

