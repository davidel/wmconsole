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




typedef struct s_WmcSkChannel {
	WmcChannel Ch;
	SOCKET SkFd;
	CRITICAL_SECTION CS;
} WmcSkChannel;




static long WmcSock_Read(void *pPrivate, void *pData, long lSize);
static long WmcSock_Write(void *pPrivate, WmcBuffer const *pBufs, int iNumBufs);
static int WmcBSlashEndedW(wchar_t const *pszPath);




int WmcMakeConsole(WmcConsole *pWmCon) {
	long i;
	RConIoctlImMaster IM;

	for (i = 0; i < 10; i++) {
		swprintf(pWmCon->szDevName, L"%s%ld:", RCON_DEV_PREFIXL, i);
		if ((pWmCon->hFile = CreateFile(pWmCon->szDevName,
						GENERIC_READ | GENERIC_WRITE,
						0, NULL, OPEN_EXISTING, 0,
						NULL)) == INVALID_HANDLE_VALUE)
			break;
		WMCM_ZERODATA(IM);
		if (DeviceIoControl(pWmCon->hFile, RCON_IOCTL_SETOWNER, (LPVOID) &IM,
				    sizeof(IM), 0, 0, 0, 0) && IM.iOwner) {
			pWmCon->hDevice = IM.hDevice;
			break;
		}
		CloseHandle(pWmCon->hFile);
	}
	if (i == 10)
		return -1;
	pWmCon->lConId = i;
	if (pWmCon->hFile == INVALID_HANDLE_VALUE) {
		if ((pWmCon->hDevice = RegisterDevice(RCON_DEV_PREFIXL, (DWORD) i,
						      L"rconsole.dll", (DWORD) i)) == NULL)
			return -1;
		if ((pWmCon->hFile = CreateFile(pWmCon->szDevName, GENERIC_READ | GENERIC_WRITE,
						0, NULL, OPEN_EXISTING, 0,
						NULL)) == INVALID_HANDLE_VALUE) {
			DeregisterDevice(pWmCon->hDevice);
			return -1;
		}
		WMCM_ZERODATA(IM);
		IM.hDevice = pWmCon->hDevice;
		DeviceIoControl(pWmCon->hFile, RCON_IOCTL_SETOWNER, (LPVOID) &IM,
				sizeof(IM), 0, 0, 0, 0);
	}

	return 0;
}

int WmcInstallConsole(WmcConsole *pWmCon) {

	SetStdioPathW(0, pWmCon->szDevName);
	SetStdioPathW(1, pWmCon->szDevName);
	SetStdioPathW(2, pWmCon->szDevName);

	return 0;
}

int WmcShutdownConsole(WmcConsole *pWmCon) {

	DeviceIoControl(pWmCon->hFile, RCON_IOCTL_SHUTDOWN, NULL, 0, 0, 0, 0, 0);

	return 0;
}

void WmcCloseConsole(WmcConsole *pWmCon, int iUnregDevice) {

	CloseHandle(pWmCon->hFile);
	if (iUnregDevice)
		DeregisterDevice(pWmCon->hDevice);
}

static long WmcSock_Read(void *pPrivate, void *pData, long lSize) {
	WmcSkChannel *pCh = (WmcSkChannel *) pPrivate;
	long lRead, lRdCur;

	for (lRead = 0; lRead < lSize;) {
		if ((lRdCur = recv(pCh->SkFd, (char *) pData + lRead,
				   (int) (lSize - lRead), 0)) <= 0)
			break;
		lRead += lRdCur;
	}

	return lRead;
}

static long WmcSock_Write(void *pPrivate, WmcBuffer const *pBufs, int iNumBufs) {
	WmcSkChannel *pCh = (WmcSkChannel *) pPrivate;
	int i;
	long lTotWrite;

	/*
	 * This is required to be atomic over the whole buffers set, so that's
	 * the reason why we need to wrap it with a lock.
	 */
	EnterCriticalSection(&pCh->CS);
	for (i = 0, lTotWrite = 0; i < iNumBufs; i++) {
		long lWrite, lWrCur;
		WmcBuffer const *pBuf = &pBufs[i];

		for (lWrite = 0; lWrite < pBuf->lSize;) {
			if ((lWrCur = send(pCh->SkFd, (char const *) pBuf->pData + lWrite,
					   (int) (pBuf->lSize - lWrite), 0)) <= 0) {
				LeaveCriticalSection(&pCh->CS);
				return -1;
			}
			lWrite += lWrCur;
		}
		lTotWrite += lWrite;
	}
	LeaveCriticalSection(&pCh->CS);

	return lTotWrite;
}

int WmcMkSockChannel(WmcChannel **ppCh, SOCKET SkFd) {
	WmcSkChannel *pCh;

	if ((pCh = (WmcSkChannel *) malloc(sizeof(WmcSkChannel))) == NULL)
		return -1;
	WMCM_ZERODATA(*pCh);
	pCh->Ch.pPrivate = pCh;
	pCh->Ch.pfnRead = WmcSock_Read;
	pCh->Ch.pfnWrite = WmcSock_Write;
	pCh->SkFd = SkFd;
	InitializeCriticalSection(&pCh->CS);
	*ppCh = &pCh->Ch;

	return 0;
}

void WmcFreeSockChannel(WmcChannel *pCh, int iCloseSk) {
	WmcSkChannel *pSkCh = (WmcSkChannel *) pCh->pPrivate;

	if (iCloseSk)
		closesocket(pSkCh->SkFd);
	DeleteCriticalSection(&pSkCh->CS);
	free(pSkCh);
}

char *WmcVSPrintf(char const *pszFmt, va_list Args) {
	char *pszStr;

	WMCM_VSPRINTF(pszStr, Args, pszFmt);

	return pszStr;
}

wchar_t *WmcVSPrintfW(wchar_t const *pszFmt, va_list Args) {
	wchar_t *pszStr;

	WMCM_VSPRINTFW(pszStr, Args, pszFmt);

	return pszStr;
}

char *WmcSPrintf(char const *pszFmt, ...) {
	char *pszStr;
	va_list Args;

	va_start(Args, pszFmt);
	pszStr = WmcVSPrintf(pszFmt, Args);
	va_end(Args);

	return pszStr;
}

wchar_t *WmcSPrintfW(wchar_t const *pszFmt, ...) {
	wchar_t *pszStr;
	va_list Args;

	va_start(Args, pszFmt);
	pszStr = WmcVSPrintfW(pszFmt, Args);
	va_end(Args);

	return pszStr;
}

int WmcChanVPrintf(WmcChannel const *pCh, int iChNbr, char const *pszFmt, va_list Args) {
	int iError;
	char *pszData;

	WMCM_VSPRINTF(pszData, Args, pszFmt);
	if (pszData == NULL)
		return -1;
	iError = WmcPktSend(pCh, iChNbr, pszData, strlen(pszData) + 1);
	free(pszData);

	return iError;
}

int WmcChanPrintf(WmcChannel const *pCh, int iChNbr, char const *pszFmt, ...) {
	int iError;
	va_list Args;

	va_start(Args, pszFmt);
	iError = WmcChanVPrintf(pCh, iChNbr, pszFmt, Args);
	va_end(Args);

	return iError;
}

wchar_t *WmcByte2Wide(char const *pszStr, long lSize, long lExtra) {
	long i;
	wchar_t *pszWStr;

	if (lSize < 0)
		lSize = strlen(pszStr);
	if ((pszWStr = (wchar_t *) malloc((lSize + lExtra + 1) * sizeof(wchar_t))) == NULL)
		return NULL;
	for (i = 0; i < lSize; i++)
		pszWStr[i] = (wchar_t) pszStr[i];
	pszWStr[i] = 0;

	return pszWStr;
}

char *WmcWide2Byte(wchar_t const *pszWStr, long lSize, long lExtra) {
	long i;
	char *pszStr;

	if (lSize < 0)
		lSize = wcslen(pszWStr);
	if ((pszStr = (char *) malloc(lSize + lExtra + 1)) == NULL)
		return NULL;
	for (i = 0; i < lSize; i++)
		pszStr[i] = (char) pszWStr[i];
	pszStr[i] = 0;

	return pszStr;
}

char *WmcGetPath(char const *pszCWD, char const *pszPath) {
	char *pszCD, *pszEOD, *pszRPath;

	/*
	 * We make things so that we allocate an extra byte at the end of
	 * the string, by adding an extra space that is removed by
	 * the WMCM_DROPLAST(). In this way the caller can safely
	 * append a backslash to the returned string.
	 */
	if (*pszPath == '\\')
		pszRPath = WmcSPrintf("%s ", pszPath);
	else if (strcmp(pszPath, ".") == 0)
		pszRPath = WmcSPrintf("%s ", pszCWD);
	else {
		if ((pszCD = _strdup(pszCWD)) == NULL)
			return NULL;
		if ((pszEOD = strrchr(pszCD, '\\')) == NULL)
			pszEOD = pszCD;
		while (strncmp(pszPath, "..", 2) == 0) {
			*pszEOD = '\0';
			if ((pszEOD = strrchr(pszCD, '\\')) != NULL)
				pszEOD[1] = '\0';
			else
				strcpy(pszCD, "\\");
			pszPath += 2;
			if (*pszPath == '\\')
				pszPath++;
			else if (*pszPath != '\0') {
				free(pszCD);
				return NULL;
			}
		}
		pszRPath = WmcSPrintf("%s%s ", pszCD, pszPath);
		free(pszCD);
	}
	if (pszRPath != NULL) {
		WMCM_DROPLAST(pszRPath, ' ');
		WMCM_DROPLAST(pszRPath, '\\');
	}

	return pszRPath;
}

wchar_t *WmcGetPathW(char const *pszCWD, char const *pszPath) {
	char *pszRPath;
	wchar_t *pszPathW;

	if ((pszRPath = WmcGetPath(pszCWD, pszPath)) == NULL)
		return NULL;
	/*
	 * Allocate one chracter extra for the reason above mentioned.
	 */
	pszPathW = WmcByte2Wide(pszRPath, -1, 1);
	free(pszRPath);

	return pszPathW;
}

int WmcGetPathInfoW(wchar_t const *pszPath, WIN32_FIND_DATA *pWFD) {
	HANDLE hFind;

	if ((hFind = FindFirstFile(pszPath, pWFD)) == INVALID_HANDLE_VALUE)
		return -1;
	FindClose(hFind);

	return 0;
}

int WmcGetPathInfo(char const *pszPath, WIN32_FIND_DATA *pWFD) {
	int iError;
	wchar_t *pszPathW;

	if ((pszPathW = WmcByte2Wide(pszPath, -1, 0)) == NULL)
		return -1;
	iError = WmcGetPathInfoW(pszPathW, pWFD);
	free(pszPathW);

	return iError;
}

int WmcEndWith(char const *pszStr, int iChar) {
	char const *pszEOS;

	return (pszEOS = strrchr(pszStr, iChar)) != NULL && pszEOS[1] == '\0';
}

char *WmcGetLsString(char const *pszPath, WIN32_FIND_DATA const *pWFD) {
	int i, iIsExec = 0, iIsRdOnly = 0;
	char *pszFPath = NULL, *pszLsStr;
	char const *pszPerm, *pszDE;
	FILETIME FTm;
	SYSTEMTIME STm;
	char szDate[64];
	char const *pszMonths[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
			"Sep", "Oct", "Nov", "Dec"
	};

	if (pszPath == NULL) {
		if ((pszFPath = WmcWide2Byte(pWFD->cFileName, -1, 0)) == NULL)
			return NULL;
		pszPath = pszFPath;
	}
	if ((i = strlen(pszPath)) >= 4 && _strnicmp(pszPath + i - 4, ".exe", 4) == 0)
		iIsExec++;
	if (pWFD->dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_INROM))
		iIsRdOnly++;
	if (iIsExec)
		pszPerm = iIsRdOnly ? "r-xr-xr-x": "rwxrwxrwx";
	else
		pszPerm = iIsRdOnly ? "r--r--r--": "rw-rw-rw-";
	pszDE = (pWFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "d": "-";
	if (!FileTimeToLocalFileTime(&pWFD->ftLastWriteTime, &FTm) ||
	    !FileTimeToSystemTime(&FTm, &STm)) {
		free(pszFPath);
		return NULL;
	}
	_snprintf(szDate, WMCM_COUNTOF(szDate), "%s %2d %02d:%02d %4d", pszMonths[STm.wMonth - 1],
		  (int) STm.wDay, (int) STm.wHour, (int) STm.wSecond, (int) STm.wYear);
	pszLsStr = WmcSPrintf("%s%s %7ld %s %s", pszDE, pszPerm, (long) pWFD->nFileSizeLow,
			      szDate, pszPath);
	free(pszFPath);

	return pszLsStr;
}

int WmcWildPath(char const *pszPath) {
	char const *pszEOD;

	return (pszEOD = strrchr(pszPath, '\\')) != NULL &&
		(strchr(pszEOD + 1, '*') != NULL || strchr(pszEOD + 1, '?') != NULL);
}

int WmcWildMatch(wchar_t const *pszStr, wchar_t const *pszMatch, int iCaseSens) {

	for (; *pszMatch != 0 && *pszStr != 0;) {
		if (*pszMatch == (wchar_t) '*') {
			for (pszMatch++; *pszMatch == (wchar_t) '*'; pszMatch++);
			if (*pszMatch == 0)
				return 1;
			for (; *pszStr != 0; pszStr++)
				if (WMCM_SAMECHARW(iCaseSens, *pszStr, *pszMatch))
					break;
			if (*pszStr == 0)
				return 0;
		} else if (*pszMatch != (wchar_t) '?'){
			if (!WMCM_SAMECHARW(iCaseSens, *pszStr, *pszMatch))
				return 0;
		}
		pszMatch++;
		pszStr++;
	}

	return *pszMatch == *pszStr;
}

static int WmcBSlashEndedW(wchar_t const *pszPath) {
	int iLen = wcslen(pszPath);

	return (iLen > 0 && pszPath[iLen - 1] == (wchar_t) '\\') ? 1: 0;
}

int WmcRmTreeW(wchar_t const *pszPath) {
	int iError, iBSE;
	HANDLE hFind;
	wchar_t *pszFPath;
	WIN32_FIND_DATA *pWFD;

	if ((pWFD = (WIN32_FIND_DATA *) malloc(sizeof(WIN32_FIND_DATA))) == NULL)
		return -1;
	iBSE = WmcBSlashEndedW(pszPath);
	if ((pszFPath = WmcSPrintfW(iBSE ? L"%s*": L"%s\\*", pszPath)) == NULL) {
		free(pWFD);
		return -1;
	}
	hFind = FindFirstFile(pszFPath, pWFD);
	free(pszFPath);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((pszFPath = WmcSPrintfW(iBSE ? L"%s%s": L"%s\\%s", pszPath,
						    pWFD->cFileName)) == NULL) {
				FindClose(hFind);
				free(pWFD);
				return -1;
			}
			SetFileAttributes(pszFPath,
					  GetFileAttributes(pszFPath) & ~FILE_ATTRIBUTE_READONLY);
			if (pWFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				iError = WmcRmTreeW(pszFPath);
			else
				iError = DeleteFile(pszFPath) ? 0: -1;
			free(pszFPath);
			if (iError < 0) {
				FindClose(hFind);
				free(pWFD);
				return iError;
			}
		} while (FindNextFile(hFind, pWFD));
		FindClose(hFind);
	}
	free(pWFD);

	return RemoveDirectory(pszPath) ? 0: -1;
}

int WmcWalkTreeW(wchar_t const *pszPath, wchar_t const *pszMatch, int iRecurse,
		 int (*pfnWalk)(void *, wchar_t const *, WIN32_FIND_DATA const *),
		 void *pPrivate) {
	int iError, iBSE;
	HANDLE hFind;
	wchar_t *pszFPath;
	WIN32_FIND_DATA *pWFD;

	if ((pWFD = (WIN32_FIND_DATA *) malloc(sizeof(WIN32_FIND_DATA))) == NULL)
		return -1;
	iBSE = WmcBSlashEndedW(pszPath);
	if ((pszFPath = WmcSPrintfW(iBSE ? L"%s*": L"%s\\*", pszPath)) == NULL) {
		free(pWFD);
		return -1;
	}
	hFind = FindFirstFile(pszFPath, pWFD);
	free(pszFPath);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((pszFPath = WmcSPrintfW(iBSE ? L"%s%s": L"%s\\%s", pszPath,
						    pWFD->cFileName)) == NULL) {
				FindClose(hFind);
				free(pWFD);
				return -1;
			}
			iError = 0;
			if (pszMatch == NULL ||
			    WmcWildMatch(pWFD->cFileName, pszMatch, 0))
				iError = (*pfnWalk)(pPrivate, pszFPath, pWFD);
			if (iError == 0 && iRecurse &&
			    (pWFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				iError = WmcWalkTreeW(pszFPath, pszMatch, 1,
						      pfnWalk, pPrivate);
			free(pszFPath);
			if (iError < 0) {
				FindClose(hFind);
				free(pWFD);
				return iError;
			}
		} while (FindNextFile(hFind, pWFD));
		FindClose(hFind);
	}
	free(pWFD);

	return 0;
}

int WmcResetDevice(void) {

	return KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL) ? 0: -1;
}

int WmcMkAllDirsW(wchar_t const *pszPath, int iIsDir) {
	int iDSize, iError;
	wchar_t *pszDName, *pszBSlash;
	wchar_t const *pszCPath;
	WIN32_FIND_DATA WFD;

	if ((pszDName = _wcsdup(pszPath)) == NULL)
		return -1;
	for (pszBSlash = pszDName;;) {
		if (WmcGetPathInfoW(pszDName, &WFD) == 0)
			break;
		if ((pszBSlash = wcsrchr(pszDName, (wchar_t) '\\')) == NULL) {
			free(pszDName);
			return -1;
		}
		*pszBSlash = '\0';
	}
	for (pszCPath = pszPath + (pszBSlash - pszDName) + 1;
	     (pszBSlash = wcschr(pszCPath, (wchar_t) '\\')) != NULL;
	     pszCPath = pszBSlash + 1) {
		iDSize = (int) (pszBSlash - pszPath);
		WMCM_ZMEMCPYW(pszDName, pszPath, iDSize);
		if (WmcGetPathInfoW(pszDName, &WFD) < 0 &&
		    !CreateDirectory(pszDName, NULL)) {
			free(pszDName);
			return -1;
		}
	}
	free(pszDName);
	iError = 0;
	if (iIsDir && WmcGetPathInfoW(pszPath, &WFD) < 0)
		iError = CreateDirectory(pszPath, NULL) ? 0: -1;

	return iError;
}

char *WmcGetOsErrorStr(DWORD dwError) {
	DWORD dwFMRes;
	wchar_t *pszMsgW = NULL;
	char *pszMsg, *pszErrMsg;

	if ((dwFMRes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				     FORMAT_MESSAGE_FROM_SYSTEM |
				     FORMAT_MESSAGE_IGNORE_INSERTS,
				     NULL, dwError, LANG_NEUTRAL,
				     (LPTSTR) &pszMsgW, 0, NULL)) == 0)
		pszErrMsg = WmcSPrintf("Unknown error (0x%lx)", (long) dwError);
	else {
		if ((pszMsg = WmcWide2Byte(pszMsgW, -1, 0)) == NULL) {
			LocalFree((HLOCAL) pszMsgW);
			return NULL;
		}
		WMCM_STRRTRIM(pszMsg, "\r\n");
		pszErrMsg = WmcSPrintf("%s (0x%lx)", pszMsg, (long) dwError);
		free(pszMsg);
	}
	if (pszMsgW != NULL)
		LocalFree((HLOCAL) pszMsgW);

	return pszErrMsg;
}

int WmcChanOSErrorReportEV(WmcChannel const *pCh, int iChNbr, DWORD dwError,
			   char const *pszFmt, va_list Args) {
	int iError;
	char *pszErrMsg;

	if ((pszErrMsg = WmcGetOsErrorStr(dwError)) != NULL) {
		iError = WmcChanPrintf(pCh, iChNbr, "OS error: %s\n", pszErrMsg);
		free(pszErrMsg);
		if (iError < 0)
			return iError;
	}
	iError = WmcChanVPrintf(pCh, iChNbr, pszFmt, Args);

	return iError < 0 ? iError: WmcPktSend(pCh, iChNbr, NULL, 0);
}

int WmcChanOSErrorReportE(WmcChannel const *pCh, int iChNbr, DWORD dwError,
			  char const *pszFmt, ...) {
	int iError;
	va_list Args;

	va_start(Args, pszFmt);
	iError = WmcChanOSErrorReportEV(pCh, iChNbr, dwError, pszFmt, Args);
	va_end(Args);

	return iError;
}

int WmcChanOSErrorReport(WmcChannel const *pCh, int iChNbr, char const *pszFmt, ...) {
	int iError;
	va_list Args;

	va_start(Args, pszFmt);
	iError = WmcChanOSErrorReportEV(pCh, iChNbr, GetLastError(), pszFmt, Args);
	va_end(Args);

	return iError;
}

unsigned long WmcHashGen(void const *pData, long lSize) {
	unsigned long ulHVal;
	unsigned char const *pPtr = (unsigned char const *) pData;

	for (ulHVal = 5381; lSize > 0; lSize--, pPtr++)
		ulHVal ^= (ulHVal << 5) + (*pPtr) + (ulHVal >> 2);

	return ulHVal;
}

int WmcSimpleDevIoctl(wchar_t const *pszDev, DWORD dwIoctl) {
	HANDLE hFile;
	BOOL bIoctlRes;

	if ((hFile = CreateFile(pszDev, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
				0, NULL)) == INVALID_HANDLE_VALUE)
		return -1;
	bIoctlRes = DeviceIoControl(hFile, dwIoctl, NULL, 0, 0, 0, 0, 0);
	CloseHandle(hFile);

	return bIoctlRes ? 0: -1;
}

void *WmcParseHexData(char const *pszValue, long *plSize) {
	unsigned int uVal;
	unsigned char *pData, *pPtr;

	if ((pData = (unsigned char *) malloc(strlen(pszValue) + 1)) == NULL)
		return NULL;
	for (pPtr = pData; *pszValue != '\0';) {
		if (sscanf(pszValue, "%2x", &uVal) != 1) {
			free(pData);
			return pData;
		}
		*pPtr++ = (unsigned char) uVal;
		pszValue += 2;
		if (*pszValue == ',')
			pszValue++;
	}
	*plSize = (long) (pPtr - pData);

	return pData;
}

void *WmcMemFind(void const *pMem, long lMemSize, void const *pFind, long lFindSize) {
	char const *pMPtr, *pMTop, *pFPtr, *pFTop;

	pMPtr = (char const *) pMem;
	pMTop = pMPtr + lMemSize;
	pFPtr = (char const *) pFind;
	pFTop = pFPtr + lFindSize;
	for (; pMPtr < pMTop; pMPtr++) {
		if (*pMPtr == *pFPtr) {
			if (++pFPtr == pFTop)
				return (void *) (pMPtr - lFindSize + 1);
		} else if (pFPtr != (char const *) pFind) {
			pMPtr -= pFPtr - (char const *) pFind;
			pFPtr = (char const *) pFind;
		}
	}

	return NULL;
}

void *WmcMemIFind(void const *pMem, long lMemSize, void const *pFind, long lFindSize) {
	char const *pMPtr, *pMTop, *pFPtr, *pFTop;

	pMPtr = (char const *) pMem;
	pMTop = pMPtr + lMemSize;
	pFPtr = (char const *) pFind;
	pFTop = pFPtr + lFindSize;
	for (; pMPtr < pMTop; pMPtr++) {
		if (tolower(*pMPtr) == tolower(*pFPtr)) {
			if (++pFPtr == pFTop)
				return (void *) (pMPtr - lFindSize + 1);
		} else if (pFPtr != (char const *) pFind) {
			pMPtr -= pFPtr - (char const *) pFind;
			pFPtr = (char const *) pFind;
		}
	}

	return NULL;
}

