/*
 *  RConsole by Davide Libenzi (Windows Mobile console driver)
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

#include <windows.h>
#include <svsutil.hxx>
#include <service.h>
#include <pkfuncs.h>
#include <stdio.h>
#include <string.h>
#include "llist.h"
#include "rconsole.h"




#define RCON_EXTC extern "C"

#if defined (DEBUG)
#define ZONE_ERROR DEBUGZONE(0)
#define ZONE_INIT DEBUGZONE(1)
#define ZONE_AUTH DEBUGZONE(6)
#define ZONE_WARN DEBUGZONE(14)
#define ZONE_VERBOSE DEBUGZONE(15)
#endif

#define RCON_BUFFER_SIZE (4 * 1024)
#define RCON_MAX_IOBS 10

#define RCON_BF_HANGUP (1 << 0)

#define RCON_ALLOC(s) malloc(s)
#define RCON_FREE(p) free(p)

#define RCONM_ZERODATA(s) memset(&(s), 0, sizeof(s))
#define RCONM_PTRENTRY(ptr, type, member) ((type *) ((char *) (ptr) - (unsigned long) (&((type *) 0)->member)))

#if _WIN32_WCE == 0x420 || _WIN32_WCE == 0x410 || _WIN32_WCE == 0x400
#define RCON_REGEVENTSRC(p, n) ((HANDLE) NULL)
#else
#define RCON_REGEVENTSRC(p, n) RegisterEventSource(p, n)
#endif




typedef struct s_RConBuffer {
	long lRefCnt;
	CRITICAL_SECTION CS;
	HANDLE hRdEvt, hWrEvt;
	unsigned long ulFlags;
	long lCount, lSize;
	long lPosR, lPosW;
	unsigned char Data[1];
} RConBuffer;

typedef struct s_RConIOB {
	HANDLE hDevice;
	HANDLE hProc;
	LList ChLst;
	RConBuffer *pInBuf, *pOutBuf;
} RConIOB;

typedef struct s_RConChannel {
	LList ChLnk;
	int iChNbr;
	HANDLE hProc;
	RConBuffer *pRdBuf, *pWrBuf;
} RConChannel;




static int RConInitIOB(RConIOB *pIOB, HANDLE hProc);
static int RConNewIOB(HANDLE hProc);
static void RConFreeIOB(int iIOB);
static int RConCreateBuffer(RConBuffer **ppRBuf, long lSize);
static void RConFreeBuffer(RConBuffer *pRBuf);
static long RConGetBuffer(RConBuffer *pRBuf);
static long RConRelBuffer(RConBuffer *pRBuf);
static int RConHangupBuffer(RConBuffer *pRBuf);
static long RConReadBuffer(RConBuffer *pRBuf, void *pData, long lSize);
static long RConWriteBuffer(RConBuffer *pRBuf, void const *pData, long lSize);
static int RConCreateChannel(RConChannel **ppRChan, int iIOB);
static void RConFreeChannel(RConChannel *pRChan);
static void RConHangupChannel(RConChannel *pRChan);
static void RConProcessExitCleanup(HANDLE hProc);




static RConIOB g_IOB[RCON_MAX_IOBS];
static HINSTANCE g_hInst;
static DWORD g_CallerExe;
static CRITICAL_SECTION g_RConCS;
static HANDLE g_hEventLog;
#if defined (DEBUG)
static DBGPARAM dpCurSettings = {
	TEXT("RConsole"), {
		TEXT("Error"), TEXT("Init"), TEXT(""), TEXT(""),
		TEXT(""), TEXT(""), TEXT("Auth"), TEXT(""),
		TEXT(""), TEXT(""), TEXT(""), TEXT(""),
		TEXT(""), TEXT(""), TEXT("Warn"), TEXT("Verbose")
	},
	0x7FDF
};
#endif






static int RConInitIOB(RConIOB *pIOB, HANDLE hProc) {

	pIOB->pInBuf = pIOB->pOutBuf = NULL;
	if (RConCreateBuffer(&pIOB->pInBuf, RCON_BUFFER_SIZE) < 0 ||
	    RConCreateBuffer(&pIOB->pOutBuf, RCON_BUFFER_SIZE) < 0) {
		RConFreeBuffer(pIOB->pInBuf);
		RConFreeBuffer(pIOB->pOutBuf);
		return -1;
	}
	pIOB->hDevice = NULL;
	pIOB->hProc = hProc;

	return 0;
}

static int RConNewIOB(HANDLE hProc) {
	int i;

	for (i = 0; i < RCON_MAX_IOBS; i++)
		if (g_IOB[i].hProc == NULL && LListEmpty(&g_IOB[i].ChLst))
			break;
	if (i == RCON_MAX_IOBS)
		return -1;
	if (RConInitIOB(&g_IOB[i], hProc) < 0)
		return -1;

	return i;
}

static void RConFreeIOB(int iIOB) {
	LList *pPos;
	RConChannel *pRChan;

	if (g_IOB[iIOB].hProc == NULL)
		return;
	for (pPos = LListFirst(&g_IOB[iIOB].ChLst); pPos != NULL;
	     pPos = LListNext(&g_IOB[iIOB].ChLst, pPos)) {
		pRChan = RCONM_PTRENTRY(pPos, RConChannel, ChLnk);
		RConHangupChannel(pRChan);
	}
	RConRelBuffer(g_IOB[iIOB].pInBuf);
	RConRelBuffer(g_IOB[iIOB].pOutBuf);
	g_IOB[iIOB].hProc = NULL;
}

static int RConCreateBuffer(RConBuffer **ppRBuf, long lSize) {
	RConBuffer *pRBuf;

	if ((pRBuf = (RConBuffer *) RCON_ALLOC(sizeof(RConBuffer) + lSize)) == NULL)
		return -1;
	RCONM_ZERODATA(*pRBuf);
	pRBuf->lRefCnt = 1;
	pRBuf->lSize = lSize;
	InitializeCriticalSection(&pRBuf->CS);
	if ((pRBuf->hRdEvt = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL ||
	    (pRBuf->hWrEvt = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL) {
		RConFreeBuffer(pRBuf);
		return -1;
	}
	*ppRBuf = pRBuf;

	return 0;
}

static void RConFreeBuffer(RConBuffer *pRBuf) {

	if (pRBuf == NULL)
		return;
	CloseHandle(pRBuf->hWrEvt);
	CloseHandle(pRBuf->hRdEvt);
	DeleteCriticalSection(&pRBuf->CS);
	RCON_FREE(pRBuf);
}

static long RConGetBuffer(RConBuffer *pRBuf) {
	long lRefCnt;

	EnterCriticalSection(&pRBuf->CS);
	lRefCnt = ++pRBuf->lRefCnt;
	LeaveCriticalSection(&pRBuf->CS);

	return lRefCnt;
}

static long RConRelBuffer(RConBuffer *pRBuf) {
	long lRefCnt;

	if (pRBuf == NULL)
		return -1;
	EnterCriticalSection(&pRBuf->CS);
	if ((lRefCnt = --pRBuf->lRefCnt) == 0) {
		LeaveCriticalSection(&pRBuf->CS);
		RConFreeBuffer(pRBuf);
	} else
		LeaveCriticalSection(&pRBuf->CS);

	return lRefCnt;
}

static int RConHangupBuffer(RConBuffer *pRBuf) {

	if (pRBuf == NULL)
		return -1;
	EnterCriticalSection(&pRBuf->CS);
	pRBuf->ulFlags |= RCON_BF_HANGUP;
	SetEvent(pRBuf->hWrEvt);
	SetEvent(pRBuf->hRdEvt);
	LeaveCriticalSection(&pRBuf->CS);

	return 0;
}

static long RConReadBuffer(RConBuffer *pRBuf, void *pData, long lSize) {
	long lRead, lCount;

	EnterCriticalSection(&pRBuf->CS);
	for (lRead = 0; lRead < lSize;) {
		while (pRBuf->lCount == 0) {
			if (pRBuf->ulFlags & RCON_BF_HANGUP) {
				LeaveCriticalSection(&pRBuf->CS);
				return lRead;
			}
			LeaveCriticalSection(&pRBuf->CS);
			if (WaitForSingleObject(pRBuf->hRdEvt,
						INFINITE) != WAIT_OBJECT_0)
				return -1;
			EnterCriticalSection(&pRBuf->CS);
		}
		if ((lCount = pRBuf->lCount) > lSize - lRead)
			lCount = lSize - lRead;
		if (lCount > pRBuf->lSize - pRBuf->lPosR)
			lCount = pRBuf->lSize - pRBuf->lPosR;
		memcpy((char *) pData + lRead, pRBuf->Data + pRBuf->lPosR, lCount);
		lRead += lCount;
		pRBuf->lCount -= lCount;
		if ((pRBuf->lPosR += lCount) == pRBuf->lSize)
			pRBuf->lPosR = 0;
		SetEvent(pRBuf->hWrEvt);
		if (pRBuf->lCount == 0)
			ResetEvent(pRBuf->hRdEvt);
	}
	LeaveCriticalSection(&pRBuf->CS);

	return lRead;
}

static long RConWriteBuffer(RConBuffer *pRBuf, void const *pData, long lSize) {
	long lWrite, lCount;

	EnterCriticalSection(&pRBuf->CS);
	for (lWrite = 0; lWrite < lSize;) {
		while (pRBuf->lCount == pRBuf->lSize) {
			if (pRBuf->ulFlags & RCON_BF_HANGUP) {
				LeaveCriticalSection(&pRBuf->CS);
				return lWrite;
			}
			LeaveCriticalSection(&pRBuf->CS);
			if (WaitForSingleObject(pRBuf->hWrEvt,
						INFINITE) != WAIT_OBJECT_0)
				return -1;
			EnterCriticalSection(&pRBuf->CS);
		}
		if ((lCount = pRBuf->lSize - pRBuf->lCount) > lSize - lWrite)
			lCount = lSize - lWrite;
		if (lCount > pRBuf->lSize - pRBuf->lPosW)
			lCount = pRBuf->lSize - pRBuf->lPosW;
		memcpy(pRBuf->Data + pRBuf->lPosW, (char *) pData + lWrite, lCount);
		lWrite += lCount;
		pRBuf->lCount += lCount;
		if ((pRBuf->lPosW += lCount) == pRBuf->lSize)
			pRBuf->lPosW = 0;
		SetEvent(pRBuf->hRdEvt);
		if (pRBuf->lCount == pRBuf->lSize)
			ResetEvent(pRBuf->hWrEvt);
	}
	LeaveCriticalSection(&pRBuf->CS);

	return lWrite;
}

static int RConCreateChannel(RConChannel **ppRChan, int iIOB) {
	RConIOB *pIOB;
	RConChannel *pRChan;

	pIOB = &g_IOB[iIOB];
	if ((pRChan = (RConChannel *) RCON_ALLOC(sizeof(RConChannel))) == NULL)
		return -1;
	RCONM_ZERODATA(*pRChan);
	LListInit(&pRChan->ChLnk);
	pRChan->hProc = GetOwnerProcess();
	pRChan->iChNbr = iIOB;
	RConGetBuffer(pRChan->pRdBuf = pIOB->pInBuf);
	RConGetBuffer(pRChan->pWrBuf = pIOB->pOutBuf);
	/*
	 * This need to be the last thing, so we can use the list-empty
	 * trick in RConFreeChannel().
	 */
	LListAddT(&pRChan->ChLnk, &pIOB->ChLst);
	*ppRChan = pRChan;

	return 0;
}

static void RConFreeChannel(RConChannel *pRChan) {
	RConIOB *pIOB;

	if (pRChan == NULL)
		return;
	RConRelBuffer(pRChan->pRdBuf);
	RConRelBuffer(pRChan->pWrBuf);
	if (!LListEmpty(&pRChan->ChLnk)) {
		LListDel(&pRChan->ChLnk);
		pIOB = &g_IOB[pRChan->iChNbr];
		if (LListEmpty(&pIOB->ChLst))
			RConFreeIOB(pRChan->iChNbr);
	}
	RCON_FREE(pRChan);
}

static void RConHangupChannel(RConChannel *pRChan) {

	RConHangupBuffer(pRChan->pRdBuf);
	RConHangupBuffer(pRChan->pWrBuf);
}

static void RConProcessExitCleanup(HANDLE hProc) {
	int i;
	LList *pPos;
	RConChannel *pRChan;

	/*
	 * This is certainly not a performing algo, but given the number
	 * of processes we are talking about, and given the rate of process
	 * creation/exit in a CE device, noone will notice any difference from
	 * an hash/tree based lookup.
	 */
	for (i = 0; i < RCON_MAX_IOBS; i++) {
		/*
		 * Cleanup eventually stale process resources.
		 */
		for (pPos = LListFirst(&g_IOB[i].ChLst); pPos != NULL;) {
			pRChan = RCONM_PTRENTRY(pPos, RConChannel, ChLnk);
			pPos = LListNext(&g_IOB[i].ChLst, pPos);
			if (pRChan->hProc == hProc)
				RConFreeChannel(pRChan);
		}
		/*
		 * The console leader/owner is quitting. Free the IOB, that will
		 * not get recycled until the last handle to it will be closed.
		 * This need to be done after the loop above.
		 */
		if (g_IOB[i].hProc == hProc)
			RConFreeIOB(i);
	}
}

RCON_EXTC BOOL WINAPI DllEntry(HANDLE hInst, DWORD dwReason, LPVOID lpvReserved) {
	int i;

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DEBUGMSG(ZONE_INIT, (TEXT("RConsole: DllEntry attach\r\n")));

		DEBUGREGISTER((HINSTANCE) hInst);
		DisableThreadLibraryCalls((HMODULE) hInst);
		g_hInst = (HINSTANCE) hInst;
		svsutil_Initialize();
		InitializeCriticalSection(&g_RConCS);
		for (i = 0; i < RCON_MAX_IOBS; i++) {
			RCONM_ZERODATA(g_IOB[i]);
			LListInit(&g_IOB[i].ChLst);
		}
		SERVICE_FIND_CALLER(g_CallerExe);
		DEBUGMSG(ZONE_INIT, (TEXT("RConsole: Caller executable = %d\r\n"),
				     g_CallerExe));
		if (g_CallerExe == SERVICE_CALLER_PROCESS_OTHER_EXE) {
			DEBUGCHK(0);
			return FALSE;
		}
		g_hEventLog = NULL;
		break;

	case DLL_PROCESS_DETACH:
		DEBUGMSG(ZONE_INIT, (TEXT("RConsole: DllEntry detach\r\n")));

		DeleteCriticalSection(&g_RConCS);
		svsutil_DeInitialize();
		break;
	}

	return TRUE;
}

RCON_EXTC DWORD RCX_Init(DWORD dwData) {
	int iIOB;

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Init(%d)\r\n"), dwData));
	EnterCriticalSection(&g_RConCS);
	if (g_hEventLog == NULL)
		g_hEventLog = RCON_REGEVENTSRC(NULL, L"RConsole");
	iIOB = RConNewIOB(GetOwnerProcess());
	LeaveCriticalSection(&g_RConCS);
	if (iIOB < 0) {
		DEBUGMSG(ZONE_ERROR, (TEXT("RConsole:RCX_Init failed to create new IOB\r\n")));
		return 0;
	}
	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Init returns (0x%X)\r\n"), iIOB));

	return (DWORD) iIOB + 1;
}

RCON_EXTC BOOL RCX_Deinit(DWORD dwData) {
	int iIOB = (int) dwData - 1;

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Deinit(0x%X)\r\n"), dwData));
	if (iIOB < 0 || iIOB >= RCON_MAX_IOBS) {
		DEBUGMSG(ZONE_ERROR, (TEXT("RConsole:RCX_Deinit bad IOB number\r\n")));
		return FALSE;
	}
	EnterCriticalSection(&g_RConCS);
	RConFreeIOB(iIOB);
	LeaveCriticalSection(&g_RConCS);

	return TRUE;
}

RCON_EXTC DWORD RCX_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode) {
	int iIOB = (int) dwData - 1;
	RConChannel *pRChan;

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Open(0x%X, 0x%X, 0x%X)\r\n"), dwData,
			     dwAccess, dwShareMode));
	EnterCriticalSection(&g_RConCS);
	if (g_IOB[iIOB].hProc == NULL &&
	    RConInitIOB(&g_IOB[iIOB], GetOwnerProcess()) < 0) {
		LeaveCriticalSection(&g_RConCS);
		return 0;
	}
	if (RConCreateChannel(&pRChan, iIOB) < 0) {
		LeaveCriticalSection(&g_RConCS);
		return 0;
	}
	LeaveCriticalSection(&g_RConCS);

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Open Exit(0x%X)\r\n"), (DWORD) pRChan));

	return (DWORD) pRChan;
}

RCON_EXTC BOOL RCX_Close(DWORD dwData) {
	RConChannel *pRChan = (RConChannel *) (void *) dwData;

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Close(0x%X)\r\n"), dwData));
	EnterCriticalSection(&g_RConCS);
	RConFreeChannel(pRChan);
	LeaveCriticalSection(&g_RConCS);

	DEBUGMSG(ZONE_INIT, (TEXT("RCX_Close Exit\r\n")));

	return TRUE;
}

RCON_EXTC DWORD RCX_Write(DWORD dwData, LPCVOID pInBuf, DWORD dwInLen) {
	RConChannel *pRChan = (RConChannel *) (void *) dwData;
	long lWrBytes;

	DEBUGMSG(ZONE_VERBOSE, (TEXT("RCX_Write(0x%X, 0x%X, %d)\r\n"),
				dwData, pInBuf, dwInLen));
	if (PSLGetCallerTrust() != OEM_CERTIFY_TRUST &&
	    (pInBuf = (LPCVOID) MapCallerPtr((void*) pInBuf, dwInLen)) == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	lWrBytes = RConWriteBuffer(pRChan->pWrBuf, pInBuf, (long) dwInLen);

	DEBUGMSG(ZONE_VERBOSE, (TEXT("RCX_Write Exit(%ld)\r\n"), lWrBytes));

	return (DWORD) lWrBytes;
}

RCON_EXTC DWORD RCX_Read(DWORD dwData, LPVOID pBuf, DWORD dwLen) {
	RConChannel *pRChan = (RConChannel *) (void *) dwData;
	long lRdBytes;

	DEBUGMSG(ZONE_VERBOSE, (TEXT("RCX_Read Enter(0x%X, 0x%X, %d)\r\n"), dwData,
				pBuf, dwLen));
	if (PSLGetCallerTrust() != OEM_CERTIFY_TRUST &&
	    (pBuf = MapCallerPtr(pBuf, dwLen)) == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	lRdBytes = RConReadBuffer(pRChan->pRdBuf, pBuf, (long) dwLen);

	DEBUGMSG(ZONE_VERBOSE, (TEXT("RCX_Read Exit(%ld)\r\n"), lRdBytes));

	return (DWORD) lRdBytes;
}

RCON_EXTC BOOL RCX_IOControl(DWORD dwData, DWORD dwCode, PBYTE pBufIn,
			     DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
			     PDWORD pdwActualOut) {
	RConChannel *pRChan = (RConChannel *) (void *) dwData;
	DEVICE_PSL_NOTIFY *pPslPacket;

	DEBUGMSG(ZONE_VERBOSE, (TEXT("RCX_IOControl(0x%X, %d, 0x%X, %d, 0x%X, %d, 0x%X)\r\n"),
				dwData, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut));
	switch (dwCode) {
	case IOCTL_PSL_NOTIFY:
		pPslPacket = (DEVICE_PSL_NOTIFY *) pBufIn;
		if (pPslPacket->dwFlags == DLL_PROCESS_EXITING) {
			EnterCriticalSection(&g_RConCS);
			RConProcessExitCleanup(pPslPacket->hProc);
			LeaveCriticalSection(&g_RConCS);
		}
		break;

	case RCON_IOCTL_SETOWNER:
		if (PSLGetCallerTrust() != OEM_CERTIFY_TRUST)
			pBufIn = (PBYTE) MapCallerPtr(pBufIn, dwLenIn);
		if (pBufIn != NULL && dwLenIn == sizeof(RConIoctlImMaster)) {
			HANDLE hProc = GetOwnerProcess();
			RConIoctlImMaster *pIIM = (RConIoctlImMaster *) pBufIn;
			RConIOB *pIOB;
			RConBuffer *pRBuf;

			pIOB = &g_IOB[pRChan->iChNbr];
			/*
			 * Make this channel to be the console sink.
			 */
			EnterCriticalSection(&g_RConCS);
			if (pIIM->hDevice != NULL)
				pIOB->hDevice = pIIM->hDevice;
			else
				pIIM->hDevice = pIOB->hDevice;
			if (pIOB->hProc == NULL || pIOB->hProc == hProc) {
				pIOB->hProc = hProc;
				pIIM->iOwner = 1;
			} else
				pIIM->iOwner = 0;
			pRBuf = pRChan->pRdBuf;
			pRChan->pRdBuf = pRChan->pWrBuf;
			pRChan->pWrBuf = pRBuf;
			LeaveCriticalSection(&g_RConCS);
		} else
			DEBUGCHK(0);
		break;

	case RCON_IOCTL_SHUTDOWN:
		RConHangupChannel(pRChan);
		break;

	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	return TRUE;
}

RCON_EXTC DWORD RCX_Seek(DWORD dwData, long pos, DWORD type) {

	DEBUGMSG(ZONE_ERROR, (TEXT("RCX_Seek(0x%X, %d, %d) NOT SUPPORTED\r\n"),
			      dwData, pos, type));

	return (DWORD) -1;
}

RCON_EXTC void RCX_PowerUp(void) {

}

RCON_EXTC void RCX_PowerDown(void) {

}

