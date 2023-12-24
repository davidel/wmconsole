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

