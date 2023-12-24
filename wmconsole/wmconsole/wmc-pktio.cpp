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

