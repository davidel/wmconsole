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




int WmcCmd_ls(WmcCtx *pCtx, char **ppszArgsA) {
	HANDLE hFind;
	char *pszLsPath, *pszTmp;
	wchar_t *pszLsPathW;
	WIN32_FIND_DATA WFD;

	if (ppszArgsA[1] != NULL) {
		if ((pszLsPath = WmcGetPath(pCtx->pszCWD, ppszArgsA[1])) == NULL) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
					  ppszArgsA[1]) < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_INVALIDARG;
		}
		if (!WmcWildPath(pszLsPath)) {
			if (WmcGetPathInfo(pszLsPath, &WFD) < 0) {
				free(pszLsPath);
				if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Path not found: '%s'\n",
						  ppszArgsA[1]) < 0 ||
				    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
					return -1;
				return WMC_ERR_NOTFOUND;
			}
			if (WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				WMCM_DROPLAST(pszLsPath, '\\');
				pszTmp = WmcSPrintf("%s\\*", pszLsPath);
				free(pszLsPath);
				pszLsPath = pszTmp;
			}
		}
	} else
		pszLsPath = WmcSPrintf("%s*", pCtx->pszCWD);
	if (pszLsPath == NULL)
		return -1;
	pszLsPathW = WmcByte2Wide(pszLsPath, -1, 0);
	free(pszLsPath);
	if (pszLsPathW == NULL)
		return -1;
	hFind = FindFirstFile(pszLsPathW, &WFD);
	free(pszLsPathW);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((pszTmp = WmcGetLsString(NULL, &WFD)) != NULL &&
			    WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszTmp) < 0) {
				free(pszTmp);
				FindClose(hFind);
				return -1;
			}
			free(pszTmp);
		} while (FindNextFile(hFind, &WFD));
		FindClose(hFind);
	} else {
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Path not found\n") < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_NOTFOUND;
		}
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

