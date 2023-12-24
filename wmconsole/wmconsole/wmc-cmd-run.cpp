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



#define WMC_DETACHED_CONSOLE L"CON0:"



typedef struct s_WmcExecLnkCtx {
	WmcCtx *pCtx;
	WmcConsole *pWmCon;
} WmcExecLnkCtx;




static wchar_t *WmcBldCmdLine(char **ppszArgsA);
static DWORD WINAPI WmcRdProcThread(LPVOID lpParam);
static int WmcExe_exec(WmcCtx *pCtx, wchar_t *pszPathW, wchar_t *pszCmdLnW);
static int WmcExe_run(WmcCtx *pCtx, wchar_t *pszPathW, wchar_t *pszCmdLnW, int iWait);




static wchar_t *WmcBldCmdLine(char **ppszArgsA) {
	int i, j, iSize, iAlloc, iLen;
	wchar_t *pszCmdLn, *pszAlloc;

	for (i = iSize = iAlloc = 0, pszCmdLn = NULL; ppszArgsA[i] != NULL; i++) {
		iLen = strlen(ppszArgsA[i]);
		if (iSize + iLen + 4 >= iAlloc) {
			iAlloc = 2 * (iSize + iLen + 4);
			if ((pszAlloc = (wchar_t *)
			     realloc(pszCmdLn, iAlloc * sizeof(wchar_t))) == NULL) {
				free(pszCmdLn);
				return NULL;
			}
			pszCmdLn = pszAlloc;
		}
		if (i > 0)
			pszCmdLn[iSize++] = (wchar_t) ' ';
		for (j = 0; j < iLen; j++)
			pszCmdLn[iSize++] = (wchar_t) ppszArgsA[i][j];
		pszCmdLn[iSize] = 0;
	}

	return pszCmdLn;
}

static DWORD WINAPI WmcRdProcThread(LPVOID lpParam) {
	WmcExecLnkCtx *pExCtx = (WmcExecLnkCtx *) lpParam;
	DWORD dwRead;
	char TmpBuf[1024];

	for (;;) {
		dwRead = 0;
		if (!ReadFile(pExCtx->pWmCon->hFile, TmpBuf, sizeof(TmpBuf),
			      &dwRead, NULL) || dwRead == 0)
			break;
		if (WmcPktSend(pExCtx->pCtx->pCh, WMC_CHAN_CONIO, TmpBuf,
			       dwRead) < 0)
			return 1;
	}

	return 0;
}

static int WmcExe_exec(WmcCtx *pCtx, wchar_t *pszPathW, wchar_t *pszCmdLnW) {
	HANDLE hRdThread;
	WmcConsole WmCon;
	WmcExecLnkCtx ExCtx;
	PROCESS_INFORMATION PI;

	if (WmcMakeConsole(&WmCon) < 0)
		return -1;
	WmcInstallConsole(&WmCon);
	WMCM_ZERODATA(PI);
	if (!CreateProcess(pszPathW, pszCmdLnW, NULL, NULL, FALSE, 0, NULL,
			   NULL, NULL, &PI)) {
		WmcCloseConsole(&WmCon, 0);
		return -1;
	}
	WMCM_ZERODATA(ExCtx);
	ExCtx.pCtx = pCtx;
	ExCtx.pWmCon = &WmCon;
	hRdThread = CreateThread(NULL, 0, WmcRdProcThread, (LPVOID) &ExCtx,
				 0, NULL);
	WaitForSingleObject(PI.hProcess, INFINITE);
	CloseHandle(PI.hThread);
	CloseHandle(PI.hProcess);
	WmcShutdownConsole(&WmCon);
	WaitForSingleObject(hRdThread, INFINITE);
	CloseHandle(hRdThread);
	WmcCloseConsole(&WmCon, 0);

	return 0;
}

static int WmcExe_run(WmcCtx *pCtx, wchar_t *pszPathW, wchar_t *pszCmdLnW, int iWait) {
	PROCESS_INFORMATION PI;

	SetStdioPathW(0, WMC_DETACHED_CONSOLE);
	SetStdioPathW(1, WMC_DETACHED_CONSOLE);
	SetStdioPathW(2, WMC_DETACHED_CONSOLE);
	WMCM_ZERODATA(PI);
	if (!CreateProcess(pszPathW, pszCmdLnW, NULL, NULL, FALSE, 0, NULL,
			   NULL, NULL, &PI))
		return -1;
	if (iWait)
		WaitForSingleObject(PI.hProcess, INFINITE);
	CloseHandle(PI.hThread);
	CloseHandle(PI.hProcess);

	return 0;
}

int WmcCmd_run(WmcCtx *pCtx, char **ppszArgsA) {
	int iError;
	wchar_t *pszPathW, *pszCmdLnW;

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[1])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	pszCmdLnW = WmcBldCmdLine(&ppszArgsA[2]);
	if (_stricmp(ppszArgsA[0], "run") == 0)
		iError = WmcExe_run(pCtx, pszPathW, pszCmdLnW, 0);
	else
		iError = WmcExe_exec(pCtx, pszPathW, pszCmdLnW);
	free(pszCmdLnW);
	free(pszPathW);
	if (iError < 0) {
		if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL, "Unable to execute: '%s'\n",
					 ppszArgsA[1]) < 0)
			return -1;
		return WMC_ERR_EXEC;
	}
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

