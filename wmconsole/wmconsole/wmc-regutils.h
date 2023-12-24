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

#if !defined(_WMC_REGUTILS_H)
#define _WMC_REGUTILS_H


#define REG_KEYTYPE ((DWORD) -1)


typedef struct s_WmcKeyInfo {
	wchar_t szName[MAX_PATH];
	DWORD dwType;
	DWORD cSubKeys;
	DWORD cValues;
	DWORD dwSize;
} WmcKeyInfo;

typedef struct s_WmcRValData {
	DWORD dwType;
	DWORD dwSize;
	void *pData;
} WmcRValData;



wchar_t *WmcRegShortPathW(wchar_t const *pszRPath);
wchar_t *WmcRegLongPathW(wchar_t const *pszRPath);
char *WmcRegGetStrValueW(HKEY hKey, wchar_t const *pszRPath);
wchar_t *WmcRegValuePos(wchar_t const *pszRPathW);
int WmcRegGetValueW(wchar_t const *pszRPathW, WmcRValData *pRVal);
void WmcRegFreeValue(WmcRValData *pRVal);
wchar_t *WmcRegVal2String(WmcRValData *pRVal);
wchar_t *WmcRegValExport(WmcRValData *pRVal);
int WmcRegOpenKeyW(HKEY *phKey, wchar_t const *pszRPathW);
int WmcRegOpenKey(HKEY *phKey, char const *pszRPath);
int WmcRegGetPathInfoW(wchar_t const *pszRPathW, WmcKeyInfo *pKInf);
int WmcRegWalkTreeW(wchar_t const *pszRPathW, wchar_t const *pszRMatchW, int iRecurse,
		    int (*pfnEnum)(void *, wchar_t const *, WmcKeyInfo const *),
		    void *pPrivate);
char *WmcRegGetLsStringW(wchar_t const *pszRPathW, WmcKeyInfo const *pKInf);
int WmcRegMkPathW(wchar_t const *pszRPath, int iIsKey, HKEY *phKey);
void *WmcRegParseData(DWORD dwType, char const *pszValue, long *plSize);

#endif

