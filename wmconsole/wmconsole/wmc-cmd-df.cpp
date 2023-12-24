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




int WmcCmd_df(WmcCtx *pCtx, char **ppszArgsA) {
	ULARGE_INTEGER uliCAvail, uliSize, uliFree;
	wchar_t *pszPathW = NULL;

	if (ppszArgsA[1] != NULL) {
		if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[1])) != NULL)
			WMCM_MISSAPPENDW(pszPathW, '\\');
	}
	if (!GetDiskFreeSpaceEx(pszPathW, &uliCAvail, &uliSize, &uliFree)) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Unable to retrieve directory stats\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_DVRSTAT;
	}
	free(pszPathW);
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "CAVB = %lu bytes\n"
			  "TOTB = %lu bytes\n"
			  "FREE = %lu bytes\n",
			  (unsigned long) uliCAvail.LowPart,
			  (unsigned long) uliSize.LowPart,
			  (unsigned long) uliFree.LowPart) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

