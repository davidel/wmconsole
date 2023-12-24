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




int WmcCmd_svc(WmcCtx *pCtx, char **ppszArgsA) {
	DWORD dwIoctl;
	wchar_t *pszDevW;

	if (ppszArgsA[1] == NULL || ppszArgsA[2] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if (strcmp(ppszArgsA[1], "-S") == 0)
		dwIoctl = IOCTL_SERVICE_START;
	else if (strcmp(ppszArgsA[1], "-T") == 0)
		dwIoctl = IOCTL_SERVICE_STOP;
	else {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid action: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((pszDevW = WmcByte2Wide(ppszArgsA[2], -1, 0)) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid argoument: '%s'\n",
				  ppszArgsA[2]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcSimpleDevIoctl(pszDevW, dwIoctl) < 0) {
		free(pszDevW);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to control service: '%s'\n",
					 ppszArgsA[2]) < 0)
			return -1;
		return WMC_ERR_SVCIOCTL;
	}
	free(pszDevW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

