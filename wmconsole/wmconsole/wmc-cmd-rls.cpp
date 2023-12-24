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




static int WmcCmdRlsEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf);




static int WmcCmdRlsEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf) {
	WmcCtx *pCtx = (WmcCtx *) pPrivate;
	int iError;
	char *pszLsStr;

	pszLsStr = WmcRegGetLsStringW(NULL, pKInf);
	iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszLsStr);
	free(pszLsStr);

	return iError;
}

int WmcCmd_rls(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	wchar_t *pszPathW;

	if (ppszArgsA[1] == NULL) {
		if ((pszPathW = WmcByte2Wide(pCtx->pszRegCWD, -1, 0)) == NULL)
			return -1;
		WMCM_DROPLASTW(pszPathW, '\\');
	} else {
		WmcKeyInfo KInf;

		if ((pszPathW = WmcGetPathW(pCtx->pszRegCWD, ppszArgsA[1])) == NULL) {
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
					  ppszArgsA[1]) < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_INVALIDARG;
		}
		if (WmcRegGetPathInfoW(pszPathW, &KInf) < 0) {
			free(pszPathW);
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Registry path not found: '%s'\n",
					  ppszArgsA[1]) < 0 ||
			    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
				return -1;
			return WMC_ERR_NOTFOUND;
		}
		if (KInf.dwType != REG_KEYTYPE) {
			char *pszLsStr;

			pszLsStr = WmcRegGetLsStringW(pszPathW, &KInf);
			iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", pszLsStr);
			free(pszLsStr);
			free(pszPathW);

			return iError < 0 ?
				iError: WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0);
		}
	}
	iError = WmcRegWalkTreeW(pszPathW, NULL, 0, WmcCmdRlsEnumCb, pCtx);
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return iError;
}

