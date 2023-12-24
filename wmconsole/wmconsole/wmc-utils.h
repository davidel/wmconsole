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

