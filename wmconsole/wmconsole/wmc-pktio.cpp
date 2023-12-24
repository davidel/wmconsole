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




int WmcPktSend(WmcChannel const *pCh, int iChNbr, void const *pData, long lSize) {
	unsigned char PktHdr[8];
	WmcBuffer DBuf[2];

	PktHdr[0] = (unsigned char) iChNbr;
	WMCM_PUT_LE16(lSize, PktHdr + 1);
	WMCM_SETDBUF(&DBuf[0], PktHdr, 3);
	WMCM_SETDBUF(&DBuf[1], pData, lSize);
	/*
	 * Note: The write callback must do an atomic write of the buffers received
	 * in input from the caller.
	 */
	if (WMCM_CH_WRITE(pCh, DBuf, lSize > 0 ? 2: 1) < 0)
		return -1;

	return 0;
}

int WmcPktRecv(WmcChannel const *pCh, int *piChNbr, void **ppData, long *plSize) {
	unsigned long ulSize;
	void *pData;
	unsigned char PktHdr[8];

	if (WMCM_CH_READ(pCh, PktHdr, 3) < 0)
		return -1;
	*piChNbr = (int) PktHdr[0];
	WMCM_GET_LE16(ulSize, PktHdr + 1);
	if ((pData = malloc(ulSize + 1)) == NULL)
		return -1;
	if (ulSize != 0 &&
	    WMCM_CH_READ(pCh, pData, (long) ulSize) != (long) ulSize) {
		free(pData);
		return -1;
	}
	((char *) pData)[ulSize] = 0;
	*ppData = pData;
	*plSize = (long) ulSize;

	return 0;
}

