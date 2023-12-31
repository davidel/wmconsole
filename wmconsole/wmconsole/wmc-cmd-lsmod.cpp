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




static int WmcListProcModules(WmcCtx *pCtx, PROCESSENTRY32 const *pPEnt);




static int WmcListProcModules(WmcCtx *pCtx, PROCESSENTRY32 const *pPEnt) {
	HANDLE hSnap;
	char *pszPath;
	MODULEENTRY32 MEnt;

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPNOHEAPS,
					      pPEnt->th32ProcessID)) == INVALID_HANDLE_VALUE) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to get modules snapshot\n") < 0)
			return -1;
		return WMC_ERR_MODSNAP;
	}
	WMCM_ZERODATA(MEnt);
	MEnt.dwSize = sizeof(MEnt);
	if (Module32First(hSnap, &MEnt)) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
				  "\tMID         MCNT  PCNT  BASE        SIZE    FILE\n") < 0) {
			CloseToolhelp32Snapshot(hSnap);
			return -1;
		}
		do {
                        if (GetModuleFileName(MEnt.hModule, MEnt.szExePath,
					      WMCM_COUNTOF(MEnt.szExePath)))
				pszPath = WmcWide2Byte(MEnt.szExePath, -1, 0);
			else
				pszPath = _strdup("????");
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
					  "\t0x%-10lx%-6lu%-6lu0x%-10lx%-8lu%s\n",
					  (unsigned long) MEnt.th32ModuleID,
					  (unsigned long) MEnt.GlblcntUsage,
					  (unsigned long) MEnt.ProccntUsage,
					  (unsigned long) MEnt.modBaseAddr,
					  (unsigned long) MEnt.modBaseSize, pszPath) < 0) {
				free(pszPath);
				CloseToolhelp32Snapshot(hSnap);
				return -1;
			}
			free(pszPath);
		} while (Module32Next(hSnap, &MEnt));
	}
	CloseToolhelp32Snapshot(hSnap);

	return 0;
}

int WmcCmd_lsmod(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	HANDLE hSnap, hProc;
	char *pszPath;
	wchar_t *pszExePath, *pszMatch = NULL;
	PROCESSENTRY32 PEnt;
	wchar_t szExePath[MAX_PATH];

	if (ppszArgsA[1] != NULL)
		pszMatch = WmcByte2Wide(ppszArgsA[1], -1, 0);
	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS,
					      0)) == INVALID_HANDLE_VALUE) {
		free(pszMatch);
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to get process snapshot\n") < 0)
			return -1;
		return WMC_ERR_PROCSNAP;
	}
	WMCM_ZERODATA(PEnt);
	PEnt.dwSize = sizeof(PEnt);
	if (Process32First(hSnap, &PEnt)) {
		do {
			pszExePath = PEnt.szExeFile;
			if ((hProc = OpenProcess(0, FALSE, PEnt.th32ProcessID)) != NULL) {
				if (GetModuleFileName((HMODULE) hProc, szExePath,
						      WMCM_COUNTOF(szExePath)) > 0)
					pszExePath = szExePath;
				CloseHandle(hProc);
			}
			if (pszMatch == NULL || WmcWildMatch(PEnt.szExeFile, pszMatch, 0) ||
			    WmcWildMatch(pszExePath, pszMatch, 0)) {
				pszPath = WmcWide2Byte(pszExePath, -1, 0);
				iError = WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
						       "PID = 0x%-10lxPPID = 0x%-10lxNTHR = %-6lu"
						       "PRIO = %-6ldFILE = '%s'\n",
						       (unsigned long) PEnt.th32ProcessID,
						       (unsigned long) PEnt.th32ParentProcessID,
						       (unsigned long) PEnt.cntThreads,
						       (long) PEnt.pcPriClassBase, pszPath);
				free(pszPath);
				if (iError < 0 ||
				    (iError = WmcListProcModules(pCtx, &PEnt)) != 0) {
					free(pszMatch);
					CloseToolhelp32Snapshot(hSnap);
					return iError;
				}
			}
		} while (Process32Next(hSnap, &PEnt));
	}
	CloseToolhelp32Snapshot(hSnap);
	free(pszMatch);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

