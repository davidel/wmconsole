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




static int WmcPrintModuleHit(WmcCtx *pCtx, PROCESSENTRY32 const *pPEnt,
			     MODULEENTRY32 const *pMEnt);
static int WmcMatchProcModule(WmcCtx *pCtx, wchar_t const *pszMatch,
			      PROCESSENTRY32 const *pPEnt);




static int WmcPrintModuleHit(WmcCtx *pCtx, PROCESSENTRY32 const *pPEnt,
			     MODULEENTRY32 const *pMEnt) {
	HANDLE hProc;
	wchar_t const *pszExePath;
	char *pszPath;
	wchar_t szExePath[MAX_PATH];

	pszExePath = pPEnt->szExeFile;
	if ((hProc = OpenProcess(0, FALSE, pPEnt->th32ProcessID)) != NULL) {
		if (GetModuleFileName((HMODULE) hProc, szExePath,
				      WMCM_COUNTOF(szExePath)) > 0)
			pszExePath = szExePath;
		CloseHandle(hProc);
	}
	pszPath = WmcWide2Byte(pszExePath, -1, 0);
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "PID = 0x%-10lxPPID = 0x%-10lxNTHR = %-6lu"
			  "PRIO = %-6ldFILE = '%s'\n",
			  (unsigned long) pPEnt->th32ProcessID,
			  (unsigned long) pPEnt->th32ParentProcessID,
			  (unsigned long) pPEnt->cntThreads,
			  (long) pPEnt->pcPriClassBase, pszPath) < 0 ||
	    WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "\tMID         MCNT  PCNT  BASE        SIZE    FILE\n") < 0) {
		free(pszPath);
		return -1;
	}
	free(pszPath);
	pszPath = WmcWide2Byte(pMEnt->szExePath, -1, 0);
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL,
			  "\t0x%-10lx%-6lu%-6lu0x%-10lx%-8lu%s\n",
			  (unsigned long) pMEnt->th32ModuleID,
			  (unsigned long) pMEnt->GlblcntUsage,
			  (unsigned long) pMEnt->ProccntUsage,
			  (unsigned long) pMEnt->modBaseAddr,
			  (unsigned long) pMEnt->modBaseSize, pszPath) < 0) {
		free(pszPath);
		return -1;
	}
	free(pszPath);

	return 0;
}

static int WmcMatchProcModule(WmcCtx *pCtx, wchar_t const *pszMatch,
			      PROCESSENTRY32 const *pPEnt) {
	int iError;
	HANDLE hSnap;
	wchar_t const *pszMName;
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
		do {
			GetModuleFileName(MEnt.hModule, MEnt.szExePath,
					  WMCM_COUNTOF(MEnt.szExePath));
			if (WmcWildMatch(MEnt.szExePath, pszMatch, 0) ||
			    ((pszMName = wcsrchr(MEnt.szExePath, (wchar_t) '\\')) != NULL &&
			     WmcWildMatch(pszMName + 1, pszMatch, 0))) {
				if ((iError = WmcPrintModuleHit(pCtx, pPEnt, &MEnt)) != 0) {
					CloseToolhelp32Snapshot(hSnap);
					return iError;
				}
			}
		} while (Module32Next(hSnap, &MEnt));
	}
	CloseToolhelp32Snapshot(hSnap);

	return 0;
}

int WmcCmd_wld(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	HANDLE hSnap;
	wchar_t *pszMatch = NULL;
	PROCESSENTRY32 PEnt;

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
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
			if ((iError = WmcMatchProcModule(pCtx, pszMatch, &PEnt)) != 0) {
				free(pszMatch);
				CloseToolhelp32Snapshot(hSnap);
				return iError;
			}
		} while (Process32Next(hSnap, &PEnt));
	}
	CloseToolhelp32Snapshot(hSnap);
	free(pszMatch);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

