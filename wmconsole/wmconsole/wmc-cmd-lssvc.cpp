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



#define WMC_SVCLIST_BUFSIZE (32 * 1024)





int WmcCmd_lssvc(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	DWORD i, dwEnts, dwSize;
	ServiceEnumInfo *pSEI;
	char *pszPrefix, *pszDllName;

	dwSize = WMC_SVCLIST_BUFSIZE;
	if ((pSEI = (ServiceEnumInfo *) malloc(dwSize)) == NULL) {
		WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Out of memory\n");
		WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0);
		return -1;
	}
	dwEnts = 0;
	if (!EnumServices((PBYTE) pSEI, &dwEnts, &dwSize)) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Cannot enumerate services\n") < 0)
			return -1;
		return WMC_ERR_SVCENUM;
	}
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "PFIX    STATUS   FILE\n") < 0) {
		free(pSEI);
		return -1;
	}
	for (i = 0; i < dwEnts; i++) {
		pszPrefix = WmcWide2Byte(pSEI[i].szPrefix, -1, 0);
		pszDllName = WmcWide2Byte(pSEI[i].szDllName, -1, 0);
		iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
				       "%-8s%-9ld%s\n",
				       pszPrefix, (long) pSEI[i].dwServiceState,
				       pszDllName);
		free(pszDllName);
		free(pszPrefix);
		if (iError < 0) {
			free(pSEI);
			return -1;
		}
	}
	free(pSEI);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

