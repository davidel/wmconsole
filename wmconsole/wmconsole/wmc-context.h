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

#if !defined(_WMC_CONTEXT_H)
#define _WMC_CONTEXT_H


typedef struct s_WmcCtx {
	WmcSvcConfig *pWcfg;
	WSADATA wsaData;
	char szRndStr[128];
	SOCKET SSkFd;
	ULONG ulHRec;
	SOCKET SkFd;
	SOCKADDR_BTH PeerBTH;
	WmcChannel *pCh;
	char *pszCWD;
	char *pszRegCWD;
} WmcCtx;



int WmcCtxCreate(WmcCtx **ppCtx, WmcSvcConfig *pWcfg);
void WmcCtxClose(WmcCtx *pCtx);
int WmcCtxSessionOpen(WmcCtx *pCtx);
int WmcCtxSessionClose(WmcCtx *pCtx);


#endif

