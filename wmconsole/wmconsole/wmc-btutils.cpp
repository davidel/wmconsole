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




int WmcRegisterService(BYTE const *pSdpRec, int iRecSize, int iChOff, int iChan,
		       ULONG *pulHRec) {
	ULONG ulSdpVer = BTH_SDP_VERSION;
	BTHNS_SETBLOB *pBtBlob;
	BLOB Blb;
	WSAQUERYSET Svc;

	*pulHRec = 0;
	if ((pBtBlob = (BTHNS_SETBLOB *) malloc(sizeof(BTHNS_SETBLOB) + iRecSize)) == NULL)
		return -1;
	memset(pBtBlob, 0, sizeof(BTHNS_SETBLOB));
	pBtBlob->pRecordHandle = pulHRec;
	pBtBlob->pSdpVersion = &ulSdpVer;
	pBtBlob->fSecurity = 0;
	pBtBlob->fOptions = 0;
	pBtBlob->ulRecordLength = iRecSize;

	memcpy(pBtBlob->pRecord, pSdpRec, iRecSize);
	pBtBlob->pRecord[iChOff] = (unsigned char) iChan;
	Blb.cbSize = sizeof(BTHNS_SETBLOB) + iRecSize - 1;
	Blb.pBlobData = (PBYTE) pBtBlob;

	WMCM_ZERODATA(Svc);
	Svc.dwSize = sizeof(Svc);
	Svc.lpBlob = &Blb;
	Svc.dwNameSpace = NS_BTH;
	if (WSASetService(&Svc, RNRSERVICE_REGISTER, 0) == SOCKET_ERROR) {
		free(pBtBlob);
		return -1;
	}
	free(pBtBlob);

	return 0;
}

int WmcUnregisterService(ULONG ulHRec) {
	ULONG ulSdpVer = BTH_SDP_VERSION;
	BTHNS_SETBLOB BtBlob;
	BLOB Blb;
	WSAQUERYSET Svc;

	WMCM_ZERODATA(BtBlob);
	BtBlob.pRecordHandle = &ulHRec;
	BtBlob.pSdpVersion = &ulSdpVer;
	Blb.cbSize = sizeof(BTHNS_SETBLOB);
	Blb.pBlobData = (PBYTE) &BtBlob;

	WMCM_ZERODATA(Svc);
	Svc.dwSize = sizeof(Svc);
	Svc.lpBlob = &Blb;
	Svc.dwNameSpace = NS_BTH;

	return WSASetService(&Svc, RNRSERVICE_DELETE, 0) == SOCKET_ERROR ? -1: 0;
}

SOCKET WmcOpenSvrConn(BYTE const *pSdpRec, int iRecSize, int iChOff, int iChan,
		      ULONG *pulHRec) {
	int iNameLen;
	SOCKET SkFd;
	SOCKADDR_BTH SkAddr;

	if ((SkFd = socket(AF_BT, SOCK_STREAM, BTHPROTO_RFCOMM)) == INVALID_SOCKET)
		return INVALID_SOCKET;
	WMCM_ZERODATA(SkAddr);
	SkAddr.addressFamily = AF_BT;
	SkAddr.port = (iChan > 0) ? iChan: 0;
	iNameLen = sizeof(SkAddr);
	if (bind(SkFd, (SOCKADDR *) &SkAddr, sizeof(SkAddr)) ||
	    getsockname(SkFd, (SOCKADDR *) &SkAddr, &iNameLen) ||
	    WmcRegisterService(pSdpRec, iRecSize, iChOff, (UCHAR) SkAddr.port,
			       pulHRec) < 0 ||
	    listen(SkFd, SOMAXCONN)) {
		closesocket(SkFd);
		return INVALID_SOCKET;
	}

	return SkFd;
}

