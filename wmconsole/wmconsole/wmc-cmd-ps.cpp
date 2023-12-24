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

