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



typedef struct s_WmcTreeCopyCtx {
	WmcCtx *pCtx;
	wchar_t const *pszDPathW, *pszSPathW;
	int iSPathLen;
} WmcTreeCopyCtx;




static int WmcFsTreeCopyWalkCB(void *pPrivate, wchar_t const *pszPathW,
			       WIN32_FIND_DATA const *pWFD);
static int WmcFsTreeCopy(WmcCtx *pCtx, wchar_t const *pszDPathW, wchar_t const *pszSPathW);





static int WmcFsTreeCopyWalkCB(void *pPrivate, wchar_t const *pszPathW,
			       WIN32_FIND_DATA const *pWFD) {
	WmcTreeCopyCtx *pTCtx = (WmcTreeCopyCtx *) pPrivate;
	int iError;
	wchar_t *pszDPathW;
	WIN32_FIND_DATA WFD;

	if ((pszDPathW = WmcSPrintfW(L"%s%s", pTCtx->pszDPathW,
				     pszPathW + pTCtx->iSPathLen)) == NULL)
		return -1;
	if (pWFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		if (WmcGetPathInfoW(pszDPathW, &WFD) < 0)
			iError = CreateDirectory(pszDPathW, NULL) ? 0: -1;
		else
			iError = 0;
	} else
		iError = CopyFile(pszPathW, pszDPathW, FALSE) ? 0: -1;
	free(pszDPathW);

	return iError;
}

static int WmcFsTreeCopy(WmcCtx *pCtx, wchar_t const *pszDPathW, wchar_t const *pszSPathW) {
	WmcTreeCopyCtx TCtx;

	WMCM_ZERODATA(TCtx);
	TCtx.pCtx = pCtx;
	TCtx.pszDPathW = pszDPathW;
	TCtx.pszSPathW = pszSPathW;
	TCtx.iSPathLen = wcslen(pszSPathW);

	return WmcWalkTreeW(pszSPathW, NULL, 1, WmcFsTreeCopyWalkCB, &TCtx);
}

int WmcCmd_cp(WmcCtx *pCtx, char **ppszArgsA) {
	wchar_t *pszSPathW, *pszDPathW, *pszNPathW;
	wchar_t const *pszFName;
	WIN32_FIND_DATA WFD;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszSPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[1])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((pszDPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[2])) == NULL) {
		free(pszSPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[2]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcGetPathInfoW(pszSPathW, &WFD) < 0) {
		free(pszDPathW);
		free(pszSPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Path not found: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_NOTFOUND;
	}
	if (WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		if (WmcGetPathInfoW(pszDPathW, &WFD) == 0) {
			if ((WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				free(pszDPathW);
				free(pszSPathW);
				if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
						  "File exist: '%s'\n",
						  ppszArgsA[2]) < 0 ||
				    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
					return -1;
				return WMC_ERR_ALREADYEXIST;
			}
			if ((pszFName = wcsrchr(pszSPathW, (wchar_t) '\\')) != NULL) {
				pszNPathW = WmcSPrintfW(L"%s\\%s", pszDPathW,
							pszFName + 1);
				free(pszDPathW);
				pszDPathW = pszNPathW;
			}
		}
		if (!CreateDirectory(pszDPathW, NULL)) {
			free(pszDPathW);
			free(pszSPathW);
			if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
						 "Unable to create directory: '%s'\n",
						 ppszArgsA[2]) < 0)
				return -1;
			return WMC_ERR_MKDIR;
		}
		if (WmcFsTreeCopy(pCtx, pszDPathW, pszSPathW) < 0) {
			free(pszDPathW);
			free(pszSPathW);
			if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
						 "Unable to copy tree: '%s' -> '%s'\n",
						 ppszArgsA[1], ppszArgsA[2]) < 0)
				return -1;
			return WMC_ERR_CPFILE;
		}
	} else {
		if (WmcGetPathInfoW(pszDPathW, &WFD) == 0 &&
		    (WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if ((pszFName = wcsrchr(pszSPathW, (wchar_t) '\\')) != NULL) {
				pszNPathW = WmcSPrintfW(L"%s\\%s", pszDPathW,
							pszFName + 1);
				free(pszDPathW);
				pszDPathW = pszNPathW;
			}
		}
		if (!CopyFile(pszSPathW, pszDPathW, FALSE)) {
			free(pszDPathW);
			free(pszSPathW);
			if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
						 "Unable to copy file: '%s' -> '%s'\n",
						 ppszArgsA[1], ppszArgsA[2]) < 0)
				return -1;
			return WMC_ERR_CPFILE;
		}
	}
	free(pszDPathW);
	free(pszSPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

