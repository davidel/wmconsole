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

#if !defined(_WMC_TYPES_H)
#define _WMC_TYPES_H


typedef struct s_WmcBuffer {
	char *pData;
	long lSize;
} WmcBuffer;

typedef struct s_WmcChannel {
	void *pPrivate;
	long (*pfnRead)(void *, void *, long);
	long (*pfnWrite)(void *, WmcBuffer const *, int);
} WmcChannel;

typedef struct s_WmcSvcConfig {
	int iChan;
} WmcSvcConfig;



#endif

