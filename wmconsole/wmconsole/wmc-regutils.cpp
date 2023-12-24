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




typedef struct s_WmcNamedKey {
	HKEY hKey;
	wchar_t const *pszName;
	int iLength;
	wchar_t const *pszLongName;
} WmcNamedKey;





static wchar_t *WmcRegEmitBinaryW(wchar_t const *pszPre, WmcRValData *pRVal);





static WmcNamedKey const NamedKeys[] = {
	{ HKEY_CLASSES_ROOT, L"HKCR", 4, L"HKEY_CLASSES_ROOT" },
	{ HKEY_CURRENT_USER, L"HKCU", 4, L"HKEY_CURRENT_USER" },
	{ HKEY_LOCAL_MACHINE, L"HKLM", 4, L"HKEY_LOCAL_MACHINE" },
	{ HKEY_USERS, L"HKUS", 4, L"HKEY_USERS" },
};




wchar_t *WmcRegShortPathW(wchar_t const *pszRPath) {
	int i, iLength;
	wchar_t const *pszNext;

	if (*pszRPath == (wchar_t) '\\')
		pszRPath++;
	if ((pszNext = wcschr(pszRPath, '\\')) == NULL)
		pszNext = pszRPath + wcslen(pszRPath);
	iLength = (int) (pszNext - pszRPath);
	for (i = 0; i < (int) WMCM_COUNTOF(NamedKeys); i++)
		if (wcsncmp(pszRPath, NamedKeys[i].pszLongName, iLength) == 0)
			break;
	if (i == (int) WMCM_COUNTOF(NamedKeys))
		return NULL;

	/*
	 * This is used to create a WmConsole registry path from a CE registry path,
	 * so the leading backslash has to be there.
	 */
	return WmcSPrintfW(L"\\%s%s", NamedKeys[i].pszName, pszNext);
}

wchar_t *WmcRegLongPathW(wchar_t const *pszRPath) {
	int i, iLength;
	wchar_t const *pszNext;

	if (*pszRPath == (wchar_t) '\\')
		pszRPath++;
	if ((pszNext = wcschr(pszRPath, '\\')) == NULL)
		pszNext = pszRPath + wcslen(pszRPath);
	iLength = (int) (pszNext - pszRPath);
	for (i = 0; i < (int) WMCM_COUNTOF(NamedKeys); i++)
		if (iLength == NamedKeys[i].iLength &&
		    wcsncmp(pszRPath, NamedKeys[i].pszName, iLength) == 0)
			break;
	if (i == (int) WMCM_COUNTOF(NamedKeys))
		return NULL;

	/*
	 * This is used to create a CE registry path from a WmConsole registry path,
	 * so the leading backslash has not to be there.
	 */
	return WmcSPrintfW(L"%s%s", NamedKeys[i].pszLongName, pszNext);
}

char *WmcRegGetStrValueW(HKEY hKey, wchar_t const *pszRPath) {
	DWORD dwSize, dwType;
	void *pRData;
	char *pszResult;

	dwSize = 0;
	if (RegQueryValueEx(hKey, pszRPath, NULL, &dwType, (LPBYTE) NULL,
			    &dwSize) != ERROR_SUCCESS)
		return NULL;
	if ((pRData = malloc(dwSize)) == NULL)
		return NULL;
	if (RegQueryValueEx(hKey, pszRPath, NULL, &dwType, (LPBYTE) pRData,
			    &dwSize) != ERROR_SUCCESS) {
		free(pRData);
		return NULL;
	}
	if (dwType == REG_SZ)
		pszResult = WmcWide2Byte((wchar_t const *) pRData, dwSize / 2, 0);
	else if (dwType == REG_DWORD)
		pszResult = WmcSPrintf("%ld", (long) *(DWORD *) pRData);
	else
		pszResult = NULL;
	free(pRData);

	return pszResult;
}

wchar_t *WmcRegValuePos(wchar_t const *pszRPathW) {
	wchar_t *pszPos = wcsstr(pszRPathW, L"\\\\");

	return pszPos != NULL ? pszPos: wcsrchr(pszRPathW, '\\');
}

int WmcRegGetValueW(wchar_t const *pszRPathW, WmcRValData *pRVal) {
	HKEY hKey;
	wchar_t const *pszName;
	wchar_t *pszKey;

	if ((pszName = WmcRegValuePos(pszRPathW)) == NULL ||
	    (pszKey = _wcsdup(pszRPathW)) == NULL)
		return -1;
	pszKey[pszName - pszRPathW] = 0;
	pszName++;
	if (WmcRegOpenKeyW(&hKey, pszKey) < 0) {
		free(pszKey);
		return -1;
	}
	free(pszKey);
	WMCM_ZERODATA(*pRVal);
	if (RegQueryValueEx(hKey, pszName, NULL, &pRVal->dwType, NULL,
			    &pRVal->dwSize) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return -1;
	}
	if ((pRVal->pData = malloc(pRVal->dwSize)) == NULL) {
		RegCloseKey(hKey);
		return -1;
	}
	if (RegQueryValueEx(hKey, pszName, NULL, &pRVal->dwType, (LPBYTE) pRVal->pData,
			    &pRVal->dwSize) != ERROR_SUCCESS) {
		free(pRVal->pData);
		RegCloseKey(hKey);
		return -1;
	}
	RegCloseKey(hKey);

	return 0;
}

void WmcRegFreeValue(WmcRValData *pRVal) {

	free(pRVal->pData);
}

static wchar_t *WmcRegEmitBinaryW(wchar_t const *pszPre, WmcRValData *pRVal) {
	DWORD i;
	wchar_t *pszBin, *pszTmp;

	if ((pszBin = (wchar_t *)
	     malloc((wcslen(pszPre) + 3 * pRVal->dwSize + 1) * sizeof(wchar_t))) == NULL)
		return NULL;
	wcscpy(pszBin, pszPre);
	for (i = 0, pszTmp = pszBin + wcslen(pszBin); i < pRVal->dwSize; i++) {
		if (i)
			wcscpy(pszTmp, L","), pszTmp++;
		swprintf(pszTmp, L"%02x", (unsigned int) ((unsigned char *) pRVal->pData)[i]);
		pszTmp += 2;
	}

	return pszBin;
}

wchar_t *WmcRegVal2String(WmcRValData *pRVal) {
	wchar_t *pszVStr;

	if (pRVal->dwType == REG_BINARY)
		pszVStr = WmcRegEmitBinaryW(L"hex:", pRVal);
	else if (pRVal->dwType == REG_DWORD)
		pszVStr = WmcSPrintfW(L"dword:%08lx", (unsigned long) *(DWORD *) pRVal->pData);
	else if (pRVal->dwType == REG_SZ || pRVal->dwType == REG_EXPAND_SZ)
		pszVStr = WmcSPrintfW(L"\"%s\"", (wchar_t *) pRVal->pData);
	else if (pRVal->dwType == REG_MULTI_SZ) {
		wchar_t *pszTmp = (wchar_t *) pRVal->pData, *pszNew;

		pszVStr = WmcSPrintfW(L"\"%s\"", pszTmp);
		for (pszTmp = pszTmp + wcslen(pszTmp) + 1; *pszTmp;) {
			pszNew = WmcSPrintfW(L"%s:\"%s\"", pszVStr, pszTmp);
			free(pszVStr);
			pszVStr = pszNew;
			pszTmp = pszTmp + wcslen(pszTmp) + 1;
		}
	} else if (pRVal->dwType == REG_RESOURCE_LIST)
			pszVStr = WmcRegEmitBinaryW(L"rsl:", pRVal);
	else if (pRVal->dwType == REG_LINK)
		pszVStr = WmcRegEmitBinaryW(L"lnk:", pRVal);
	else
		pszVStr = WmcRegEmitBinaryW(L"???:", pRVal);

	return pszVStr;
}

wchar_t *WmcRegValExport(WmcRValData *pRVal) {
	wchar_t *pszVStr;

	if (pRVal->dwType == REG_BINARY)
		pszVStr = WmcRegEmitBinaryW(L"hex:", pRVal);
	else if (pRVal->dwType == REG_DWORD)
		pszVStr = WmcSPrintfW(L"dword:%08lx", (unsigned long) *(DWORD *) pRVal->pData);
	else if (pRVal->dwType == REG_SZ)
		pszVStr = WmcSPrintfW(L"\"%s\"", (wchar_t *) pRVal->pData);
	else if (pRVal->dwType == REG_EXPAND_SZ)
		pszVStr = WmcRegEmitBinaryW(L"hex(2):", pRVal);
	else if (pRVal->dwType == REG_MULTI_SZ)
		pszVStr = WmcRegEmitBinaryW(L"hex(7):", pRVal);
	else
		pszVStr = WmcRegEmitBinaryW(L"hex(?):", pRVal);

	return pszVStr;
}

int WmcRegOpenKeyW(HKEY *phKey, wchar_t const *pszRPathW) {
	int i, iError = 0;

	if (*pszRPathW == (wchar_t) '\\')
		pszRPathW++;
	for (i = 0; i < (int) WMCM_COUNTOF(NamedKeys); i++) {
		if (wcsncmp(pszRPathW, NamedKeys[i].pszName, NamedKeys[i].iLength) == 0 &&
		    (pszRPathW[NamedKeys[i].iLength] == 0 ||
		     pszRPathW[NamedKeys[i].iLength] == (wchar_t) '\\'))
			break;
	}
	if (i == (int) WMCM_COUNTOF(NamedKeys))
		return -1;
	if (pszRPathW[NamedKeys[i].iLength] == 0)
		*phKey = NamedKeys[i].hKey;
	else
		iError = (RegOpenKeyEx(NamedKeys[i].hKey, pszRPathW + NamedKeys[i].iLength + 1,
				       0, 0, phKey) == ERROR_SUCCESS) ? 0: -1;

	return iError;
}

int WmcRegOpenKey(HKEY *phKey, char const *pszRPath) {
	int iError;
	wchar_t *pszRPathW;

	if ((pszRPathW = WmcByte2Wide(pszRPath, -1, 0)) == NULL)
		return -1;
	iError = WmcRegOpenKeyW(phKey, pszRPathW);
	free(pszRPathW);

	return iError;
}

int WmcRegGetPathInfoW(wchar_t const *pszRPathW, WmcKeyInfo *pKInf) {
	HKEY hKey;
	wchar_t const *pszName;
	wchar_t *pszKey;

	if ((pszName = WmcRegValuePos(pszRPathW)) == NULL)
		return -1;
	WMCM_ZERODATA(*pKInf);
	wcscpy(pKInf->szName, pszName + 1);
	if (WmcRegOpenKeyW(&hKey, pszRPathW) == 0) {
		pKInf->dwType = REG_KEYTYPE;
		if (RegQueryInfoKey(hKey, NULL, NULL, NULL,
				    &pKInf->cSubKeys, NULL, NULL, &pKInf->cValues,
				    NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return -1;
		}
		RegCloseKey(hKey);
	} else {
		if ((pszKey = _wcsdup(pszRPathW)) == NULL)
			return -1;
		pszKey[pszName - pszRPathW] = 0;
		pszName++;
		if (WmcRegOpenKeyW(&hKey, pszKey) < 0) {
			free(pszKey);
			return -1;
		}
		free(pszKey);
		if (RegQueryValueEx(hKey, pszName, NULL, &pKInf->dwType, NULL,
				    &pKInf->dwSize) != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return -1;
		}
		RegCloseKey(hKey);
	}

	return 0;
}

int WmcRegWalkTreeW(wchar_t const *pszRPathW, wchar_t const *pszRMatchW, int iRecurse,
		    int (*pfnEnum)(void *, wchar_t const *, WmcKeyInfo const *),
		    void *pPrivate) {
	int iError;
	DWORD i, cbName, dwType;
	HKEY hKey, hSKey;
	wchar_t *pszFPath;
	WmcKeyInfo KInf;

	if (wcslen(pszRPathW) == 0) {
		for (i = 0; i < WMCM_COUNTOF(NamedKeys); i++) {
			WMCM_ZERODATA(KInf);
			if ((pszFPath = WmcSPrintfW(L"\\%s", NamedKeys[i].pszName)) < 0)
				return -1;
			if (pszRMatchW == NULL ||
			    WmcWildMatch(NamedKeys[i].pszName, pszRMatchW, 0)) {
				wcscpy(KInf.szName, NamedKeys[i].pszName);
				KInf.dwType = REG_KEYTYPE;
				if (RegQueryInfoKey(NamedKeys[i].hKey, NULL, NULL, NULL,
						    &KInf.cSubKeys, NULL, NULL, &KInf.cValues,
						    NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
					if ((iError = (*pfnEnum)(pPrivate, pszFPath, &KInf)) < 0) {
						free(pszFPath);
						return iError;
					}
				}

			}
			iError = iRecurse ? WmcRegWalkTreeW(pszFPath, pszRMatchW, 1,
							    pfnEnum, pPrivate): 0;
			free(pszFPath);
			if (iError < 0)
				return iError;
		}
	} else {
		if (WmcRegOpenKeyW(&hKey, pszRPathW) < 0)
			return -1;
		for (i = 0;; i++) {
			WMCM_ZERODATA(KInf);
			cbName = WMCM_COUNTOF(KInf.szName);
			if (RegEnumValue(hKey, i, KInf.szName, &cbName, NULL, &dwType,
					 NULL, NULL) != ERROR_SUCCESS)
				break;
			if ((pszFPath = WmcSPrintfW(L"%s\\%s", pszRPathW, KInf.szName)) < 0) {
				RegCloseKey(hKey);
				return -1;
			}
			KInf.dwType = dwType;
			if ((pszRMatchW == NULL ||
			     WmcWildMatch(KInf.szName, pszRMatchW, 0)) &&
			    RegQueryValueEx(hKey, KInf.szName, NULL, &dwType, NULL,
					    &KInf.dwSize) == ERROR_SUCCESS) {
				if ((iError = (*pfnEnum)(pPrivate, pszFPath, &KInf)) < 0) {
					RegCloseKey(hKey);
					free(pszFPath);
					return iError;
				}
			}
			free(pszFPath);
		}
		for (i = 0;; i++) {
			WMCM_ZERODATA(KInf);
			cbName = WMCM_COUNTOF(KInf.szName);
			if (RegEnumKeyEx(hKey, i, KInf.szName, &cbName, NULL, NULL,
					 NULL, NULL) != ERROR_SUCCESS)
				break;
			if ((pszFPath = WmcSPrintfW(L"%s\\%s", pszRPathW, KInf.szName)) < 0) {
				RegCloseKey(hKey);
				return -1;
			}
			if ((pszRMatchW == NULL ||
			     WmcWildMatch(KInf.szName, pszRMatchW, 0)) &&
			    WmcRegOpenKeyW(&hSKey, pszFPath) == 0) {
				KInf.dwType = REG_KEYTYPE;
				if (RegQueryInfoKey(hSKey, NULL, NULL, NULL,
						    &KInf.cSubKeys, NULL, NULL, &KInf.cValues,
						    NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
					if ((iError = (*pfnEnum)(pPrivate, pszFPath, &KInf)) < 0) {
						RegCloseKey(hSKey);
						RegCloseKey(hKey);
						free(pszFPath);
						return iError;
					}
				}
				RegCloseKey(hSKey);
			}
			iError = iRecurse ? WmcRegWalkTreeW(pszFPath, pszRMatchW, 1,
							    pfnEnum, pPrivate): 0;
			free(pszFPath);
			if (iError < 0) {
				RegCloseKey(hKey);
				return iError;
			}
		}
		RegCloseKey(hKey);
	}

	return 0;
}

char *WmcRegGetLsStringW(wchar_t const *pszRPathW, WmcKeyInfo const *pKInf) {
	char *pszName, *pszLsStr;
	char const *pszType;

	if (pszRPathW != NULL)
		pszName = WmcWide2Byte(pszRPathW, -1, 0);
	else
		pszName = WmcWide2Byte(pKInf->szName, -1, 0);
	if (pKInf->dwType == REG_KEYTYPE)
		pszLsStr = WmcSPrintf("%5s%6lu %s", "+",
				      (unsigned long) (pKInf->cSubKeys + pKInf->cValues),
				      pszName);
	else {
		if (pKInf->dwType == REG_BINARY)
			pszType = "BIN";
		else if (pKInf->dwType == REG_DWORD)
			pszType = "DW";
		else if (pKInf->dwType == REG_SZ)
			pszType = "SZ";
		else if (pKInf->dwType == REG_MULTI_SZ)
			pszType = "MSZ";
		else if (pKInf->dwType == REG_EXPAND_SZ)
			pszType = "XSZ";
		else if (pKInf->dwType == REG_RESOURCE_LIST)
			pszType = "RSL";
		else if (pKInf->dwType == REG_LINK)
			pszType = "LNK";
		else
			pszType = "???";
		pszLsStr = WmcSPrintf("%5s%6lu %s", pszType,
				      (unsigned long) pKInf->dwSize, pszName);
	}
	free(pszName);

	return pszLsStr;
}

int WmcRegMkPathW(wchar_t const *pszRPath, int iIsKey, HKEY *phKey) {
	int iDSize;
	HKEY hKey, hSubKey;
	DWORD dwDisp;
	LONG lResult;
	wchar_t *pszDName, *pszBSlash;
	wchar_t const *pszCPath;

	if ((pszDName = _wcsdup(pszRPath)) == NULL)
		return -1;
	for (pszBSlash = pszDName;;) {
		if (WmcRegOpenKeyW(&hKey, pszDName) == 0)
			break;
		if ((pszBSlash = WmcRegValuePos(pszDName)) == NULL) {
			free(pszDName);
			return -1;
		}
		*pszBSlash = '\0';
	}
	for (pszCPath = pszRPath + (pszBSlash - pszDName) + 1;
	     (pszBSlash = wcschr(pszCPath, (wchar_t) '\\')) != NULL;
	     pszCPath = pszBSlash + 1) {
		iDSize = (int) (pszBSlash - pszCPath);
		WMCM_ZMEMCPYW(pszDName, pszCPath, iDSize);
		lResult = RegCreateKeyEx(hKey, pszDName, 0, NULL, REG_OPTION_NON_VOLATILE,
					 0, NULL, &hSubKey, &dwDisp);
		RegCloseKey(hKey);
		if (lResult != ERROR_SUCCESS) {
			free(pszDName);
			return -1;
		}
		hKey = hSubKey;
	}
	if (iIsKey) {
		lResult = RegCreateKeyEx(hKey, pszCPath, 0, NULL, REG_OPTION_NON_VOLATILE,
					 0, NULL, &hSubKey, &dwDisp);
		RegCloseKey(hKey);
		if (lResult != ERROR_SUCCESS) {
			free(pszDName);
			return -1;
		}
		hKey = hSubKey;
	}
	free(pszDName);
	if (phKey != NULL)
		*phKey = hKey;
	else
		RegCloseKey(hSubKey);

	return 0;
}

void *WmcRegParseData(DWORD dwType, char const *pszValue, long *plSize) {
	char const *pszTmp;
	void *pData = NULL;

	if (dwType == REG_SZ) {
		pszTmp = pszValue + strlen(pszValue);
		if (*pszValue == '"') {
			pszValue++;
			if (*--pszTmp != '"' || pszTmp < pszValue)
				return NULL;
		}
		pData = WmcByte2Wide(pszValue, pszTmp - pszValue, 0);
		*plSize = ((long) (pszTmp - pszValue) + 1) * sizeof(wchar_t);
	} else if (dwType == REG_DWORD) {
		if ((pData = malloc(sizeof(DWORD))) == NULL)
			return NULL;
		*(DWORD *) pData = strtoul(pszValue, NULL, 0);
		*plSize = sizeof(DWORD);
	} else if (dwType == REG_BINARY) {
		pData = WmcParseHexData(pszValue, plSize);
	}

	return pData;
}

