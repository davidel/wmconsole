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




int WmcCmd_ps(WmcCtx *pCtx, char **ppszArgsA) {
	HANDLE hSnap, hProc;
	char *pszPath;
	wchar_t *pszExePath;
	PROCESSENTRY32 PEnt;
	wchar_t szExePath[MAX_PATH];

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS,
					      0)) == INVALID_HANDLE_VALUE) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
					 "Unable to get process snapshot\n") < 0)
			return -1;
		return WMC_ERR_PROCSNAP;
	}
	WMCM_ZERODATA(PEnt);
	PEnt.dwSize = sizeof(PEnt);
	if (Process32First(hSnap, &PEnt)) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
				  "PID         PPID        NTHR  PRIO  FILE\n") < 0) {
			CloseToolhelp32Snapshot(hSnap);
			return -1;
		}
		do {
			pszExePath = PEnt.szExeFile;
			if ((hProc = OpenProcess(0, FALSE, PEnt.th32ProcessID)) != NULL) {
				if (GetModuleFileName((HMODULE) hProc, szExePath,
						      WMCM_COUNTOF(szExePath)) > 0)
					pszExePath = szExePath;
				CloseHandle(hProc);
			}
			pszPath = WmcWide2Byte(pszExePath, -1, 0);
			if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
					  "0x%-10lx0x%-10lx%-6lu%-6ld%s\n",
					  (unsigned long) PEnt.th32ProcessID,
					  (unsigned long) PEnt.th32ParentProcessID,
					  (unsigned long) PEnt.cntThreads,
					  (long) PEnt.pcPriClassBase, pszPath) < 0) {
				free(pszPath);
				CloseToolhelp32Snapshot(hSnap);
				return -1;
			}
			free(pszPath);
		} while (Process32Next(hSnap, &PEnt));
	}
	CloseToolhelp32Snapshot(hSnap);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

