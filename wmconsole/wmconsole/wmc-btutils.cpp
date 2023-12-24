//    Copyright 2023 Davide Libenzi
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// 


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

