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




int WmcCmd_mem(WmcCtx *pCtx, char **ppszArgsA) {
	MEMORYSTATUS MemStat;

	WMCM_ZERODATA(MemStat);
	MemStat.dwLength = sizeof(MemStat);
	GlobalMemoryStatus(&MemStat);
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "TOTB = %lu bytes\n"
			  "FREE = %lu bytes\n",
			  (unsigned long) MemStat.dwTotalPhys,
			  (unsigned long) MemStat.dwAvailPhys) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

