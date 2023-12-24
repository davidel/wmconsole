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




typedef struct s_WmcRegDigCtx {
	WmcCtx *pCtx;
	int iCaseSens;
	DWORD dwType;
	void *pData;
	long lSize;
} WmcRegDigCtx;




static int WmcPrintRegent(WmcCtx *pCtx, wchar_t const *pszPathW, WmcKeyInfo const *pKInf);
static int WmcCmdRdigEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf);





static int WmcPrintRegent(WmcCtx *pCtx, wchar_t const *pszPathW, WmcKeyInfo const *pKInf) {
	int iError;
	char *pszLsStr;

	pszLsStr = WmcRegGetLsStringW(pszPathW, pKInf);
	iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszLsStr);
	free(pszLsStr);

	return iError;
}

static int WmcCmdRdigEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf) {
	WmcRegDigCtx *pDCtx = (WmcRegDigCtx *) pPrivate;
	int iError = 0;
	WmcRValData RVal;

	if (pKInf->dwType == REG_KEYTYPE)
		return 0;
	if (pKInf->dwType == REG_DWORD) {
		if (pDCtx->dwType == REG_DWORD &&
		    WmcRegGetValueW(pszPathW, &RVal) == 0) {
			if (memcmp(RVal.pData, pDCtx->pData, sizeof(DWORD)) == 0)
				iError = WmcPrintRegent(pDCtx->pCtx, pszPathW, pKInf);
			WmcRegFreeValue(&RVal);
		}

		return iError;
	}
	if (pDCtx->dwType == REG_DWORD)
		return 0;
	if (WmcRegGetValueW(pszPathW, &RVal) < 0)
		return -1;
	if ((pDCtx->iCaseSens && WmcMemFind(RVal.pData, (long) RVal.dwSize,
					    pDCtx->pData, pDCtx->lSize) != NULL) ||
	    (!pDCtx->iCaseSens && WmcMemIFind(RVal.pData, (long) RVal.dwSize,
					      pDCtx->pData, pDCtx->lSize) != NULL))
		iError = WmcPrintRegent(pDCtx->pCtx, pszPathW, pKInf);
	WmcRegFreeValue(&RVal);

	return iError;
}

int WmcCmd_rdig(WmcCtx *pCtx, char **ppszArgsA) {
	int iArgN, iCaseSens = 0, iError;
	wchar_t *pszRPathW;
	WmcRegDigCtx DCtx;

	WMCM_ZERODATA(DCtx);
	DCtx.pCtx = pCtx;
	DCtx.iCaseSens = 1;
	for (iArgN = 1; ppszArgsA[iArgN] != NULL; iArgN++) {
		if (strcmp(ppszArgsA[iArgN], "-i") == 0)
			DCtx.iCaseSens = 0;
		else
			break;
	}
	if (ppszArgsA[iArgN] == NULL || ppszArgsA[iArgN + 1] == NULL ||
	    ppszArgsA[iArgN + 2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (_stricmp(ppszArgsA[iArgN + 1], "BI") == 0)
		DCtx.dwType = REG_BINARY;
	else if (_stricmp(ppszArgsA[iArgN + 1], "SZ") == 0)
		DCtx.dwType = REG_SZ;
	else if (_stricmp(ppszArgsA[iArgN + 1], "DW") == 0)
		DCtx.dwType = REG_DWORD;
	else {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid data type: '%s'\n",
				  ppszArgsA[iArgN + 1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszRPathW = WmcGetPathW(pCtx->pszRegCWD, ppszArgsA[iArgN])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[iArgN]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((DCtx.pData = WmcRegParseData(DCtx.dwType, ppszArgsA[iArgN + 2],
					  &DCtx.lSize)) == NULL) {
		free(pszRPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid data format: '%s'\n",
				  ppszArgsA[3]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	/*
	 * The WmcRegParseData adds also the size of the final (zero) wide char, in case
	 * of a string. So we need to chop it off, when we're using such data for
	 * a search operation.
	 */
	if (DCtx.dwType == REG_SZ)
		DCtx.lSize -= sizeof(wchar_t);
	iError = WmcRegWalkTreeW(pszRPathW, NULL, 1, WmcCmdRdigEnumCb, &DCtx);
	free(DCtx.pData);
	free(pszRPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return iError;
}

