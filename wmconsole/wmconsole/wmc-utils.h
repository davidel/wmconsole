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


#if !defined(_WMC_UTILS_H)
#define _WMC_UTILS_H


typedef struct s_WmcConsole {
	HANDLE hFile;
	HANDLE hDevice;
	long lConId;
	wchar_t szDevName[8];
} WmcConsole;


int WmcMakeConsole(WmcConsole *pWmCon);
int WmcInstallConsole(WmcConsole *pWmCon);
int WmcShutdownConsole(WmcConsole *pWmCon);
void WmcCloseConsole(WmcConsole *pWmCon, int iUnregDevice);
int WmcMkSockChannel(WmcChannel **ppCh, SOCKET SkFd);
void WmcFreeSockChannel(WmcChannel *pCh, int iCloseSk);
char *WmcVSPrintf(char const *pszFmt, va_list Args);
wchar_t *WmcVSPrintfW(wchar_t const *pszFmt, va_list Args);
char *WmcSPrintf(char const *pszFmt, ...);
wchar_t *WmcSPrintfW(wchar_t const *pszFmt, ...);
int WmcChanVPrintf(WmcChannel const *pCh, int iChNbr, char const *pszFmt, va_list Args);
int WmcChanPrintf(WmcChannel const *pCh, int iChNbr, char const *pszFmt, ...);
wchar_t *WmcByte2Wide(char const *pszStr, long lSize, long lExtra);
char *WmcWide2Byte(wchar_t const *pszWStr, long lSize, long lExtra);
char *WmcGetPath(char const *pszCWD, char const *pszPath);
wchar_t *WmcGetPathW(char const *pszCWD, char const *pszPath);
int WmcGetPathInfoW(wchar_t const *pszPath, WIN32_FIND_DATA *pWFD);
int WmcGetPathInfo(char const *pszPath, WIN32_FIND_DATA *pWFD);
int WmcEndWith(char const *pszStr, int iChar);
char *WmcGetLsString(char const *pszPath, WIN32_FIND_DATA const *pWFD);
int WmcWildPath(char const *pszPath);
int WmcWildMatch(wchar_t const *pszStr, wchar_t const *pszMatch, int iCaseSens);
int WmcRmTreeW(wchar_t const *pszPath);
int WmcWalkTreeW(wchar_t const *pszPath, wchar_t const *pszMatch, int iRecurse,
		 int (*pfnWalk)(void *, wchar_t const *, WIN32_FIND_DATA const *),
		 void *pPrivate);
int WmcResetDevice(void);
int WmcMkAllDirsW(wchar_t const *pszPath, int iIsDir);
char *WmcGetOsErrorStr(DWORD dwError);
int WmcChanOSErrorReportEV(WmcChannel const *pCh, int iChNbr, DWORD dwError,
			   char const *pszFmt, va_list Args);
int WmcChanOSErrorReportE(WmcChannel const *pCh, int iChNbr, DWORD dwError,
			  char const *pszFmt, ...);
int WmcChanOSErrorReport(WmcChannel const *pCh, int iChNbr, char const *pszFmt, ...);
unsigned long WmcHashGen(void const *pData, long lSize);
int WmcSimpleDevIoctl(wchar_t const *pszDev, DWORD dwIoctl);
void *WmcParseHexData(char const *pszValue, long *plSize);
void *WmcMemFind(void const *pMem, long lMemSize, void const *pFind, long lFindSize);
void *WmcMemIFind(void const *pMem, long lMemSize, void const *pFind, long lFindSize);

#endif

