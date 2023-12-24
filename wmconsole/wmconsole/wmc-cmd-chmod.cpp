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




int WmcCmd_chmod(WmcCtx *pCtx, char **ppszArgsA) {
	DWORD dwAttrMask = 0, dwAttrMaskN = 0, dwAttr;
	char const *pszAttr;
	wchar_t *pszPathW = NULL;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (ppszArgsA[1][0] != '-' && ppszArgsA[1][0] != '+') {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Wrong attribute set {'+', '-'}[hsw]+\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	for (pszAttr = ppszArgsA[1] + 1; *pszAttr != '\0'; pszAttr++) {
		switch (*pszAttr) {
		case 's':
			dwAttrMask |= FILE_ATTRIBUTE_SYSTEM;
			break;
		case 'h':
			dwAttrMask |= FILE_ATTRIBUTE_HIDDEN;
			break;
		case 'w':
			dwAttrMaskN |= FILE_ATTRIBUTE_READONLY;
			break;
		}
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[2])) == NULL ||
	    (dwAttr = GetFileAttributes(pszPathW)) == (DWORD) -1) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[2]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (ppszArgsA[1][0] == '-') {
		dwAttr &= ~dwAttrMask;
		dwAttr |= dwAttrMaskN;
	} else {
		dwAttr |= dwAttrMask;
		dwAttr &= ~dwAttrMaskN;
	}
	if (!SetFileAttributes(pszPathW, dwAttr)) {
		free(pszPathW);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to set attributes: '%s'\n",
					 ppszArgsA[2]) < 0)
			return -1;
		return WMC_ERR_MKDIR;
	}
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

