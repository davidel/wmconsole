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




typedef struct s_WmcSvcCmd {
	char const *pszCmd;
	int (*pfnCmdProc)(WmcCtx *, char **);
} WmcSvcCmd;





static void WmcInitConfig(WmcSvcConfig *pWcfg);
static int WmcParseConfig(WmcSvcConfig *pWcfg, int iArgc, char const **ppszArgs);
static char **WmcGetCmd(WmcCtx *pCtx);
static void WmcFreeCmd(char **ppszCmdA);
static int WmcHandleCmd(WmcCtx *pCtx, char **ppszCmdA);
static int WmcHandleSession(WmcCtx *pCtx);
static char *WmcGetUserPasswd(WmcCtx *pCtx, char const *pszUser);
static int WmcVerifyLogin(WmcCtx *pCtx, char const *pszUser, char const *pszPwdSha);
static int WmcLoginSession(WmcCtx *pCtx);




static WmcSvcCmd const Cmds[] = {
	{ "exit", WmcCmd_exit },
	{ "shutdown", WmcCmd_shutdown },
	{ "reboot", WmcCmd_reboot },
	{ "help", WmcCmd_help },
	{ "cd", WmcCmd_cd },
	{ "pwd", WmcCmd_pwd },
	{ "mkdir", WmcCmd_mkdir },
	{ "rmdir", WmcCmd_rmdir },
	{ "ls", WmcCmd_ls },
	{ "rm", WmcCmd_rm },
	{ "mv", WmcCmd_mv },
	{ "cp", WmcCmd_cp },
	{ "chmod", WmcCmd_chmod },
	{ "rmtree", WmcCmd_rmtree },
	{ "find", WmcCmd_find },
	{ "put", WmcCmd_put },
	{ "putf", WmcCmd_put },
	{ "get", WmcCmd_get },
	{ "ps", WmcCmd_ps },
	{ "lsmod", WmcCmd_lsmod },
	{ "wld", WmcCmd_wld },
	{ "kill", WmcCmd_kill },
	{ "run", WmcCmd_run },
	{ "exec", WmcCmd_run },
	{ "df", WmcCmd_df },
	{ "mem", WmcCmd_mem },
	{ "lssvc", WmcCmd_lssvc },
	{ "lsdev", WmcCmd_lsdev },
	{ "svc", WmcCmd_svc },
	{ "rpwd", WmcCmd_rpwd },
	{ "rcd", WmcCmd_rcd },
	{ "rls", WmcCmd_rls },
	{ "rcat", WmcCmd_rcat },
	{ "rfind", WmcCmd_rfind },
	{ "rrm", WmcCmd_rrm },
	{ "rrmtree", WmcCmd_rrmtree },
	{ "rmkkey", WmcCmd_rmkkey },
	{ "rset", WmcCmd_rset },
	{ "rexp", WmcCmd_rexp },
	{ "rimp", WmcCmd_rimp },
	{ "rdig", WmcCmd_rdig },
};





static void WmcInitConfig(WmcSvcConfig *pWcfg) {
	char *pszRVal;

	WMCM_ZERODATA(*pWcfg);
	pWcfg->iChan = WMC_STD_CHANNEL;
	if ((pszRVal = WmcRegGetStrValueW(HKEY_LOCAL_MACHINE,
					  WMC_REGPATH L"Channel")) != NULL) {
		pWcfg->iChan = atoi(pszRVal);
		free(pszRVal);
	}
}

static int WmcParseConfig(WmcSvcConfig *pWcfg, int iArgc, char const **ppszArgs) {
	int i;

	WmcInitConfig(pWcfg);
	for (i = 1; i < iArgc; i++) {
		if (strcmp(ppszArgs[i], "-P") == 0) {
			if (++i < iArgc)
				pWcfg->iChan = atoi(ppszArgs[i]);
		}
	}

	return 0;
}

static char **WmcGetCmd(WmcCtx *pCtx) {
	int iChNbr, iCount, iAlloc;
	long lPktSize;
	void *pPktData;
	char *pszCmd, *pszTmp, *pszD, *pszS;
	char **ppszCmdA, **ppszCmdAn;

	if (WmcPktRecv(pCtx->pCh, &iChNbr, &pPktData, &lPktSize) < 0)
		return NULL;
	for (iCount = iAlloc = 0, ppszCmdA = NULL, pszCmd = (char *) pPktData;
	     *pszCmd != 0;) {
		if (iCount + 1 >= iAlloc) {
			iAlloc = 2 * iAlloc + 8;
			if ((ppszCmdAn = (char **)
			     realloc(ppszCmdA, iAlloc * sizeof(char *))) == NULL) {
				for (iCount--; iCount >= 0; iCount--)
					free(ppszCmdA[iCount]);
				free(ppszCmdA);
				free(pPktData);
				return NULL;
			}
			ppszCmdA = ppszCmdAn;
		}
		for (; *pszCmd == ' '; pszCmd++);
		if (*pszCmd != '"') {
			for (pszTmp = pszCmd; *pszTmp != 0; pszTmp++)
				if (*pszTmp == ' ' &&
				    (pszTmp - 1 == pszCmd || pszTmp[-1] != '\\'))
					break;
		} else {
			for (pszTmp = ++pszCmd; *pszTmp != 0; pszTmp++)
				if (*pszTmp == '"' &&
				    (pszTmp - 1 == pszCmd || pszTmp[-1] != '\\'))
					break;
		}
		if ((ppszCmdA[iCount] = (char *)
		     malloc(pszTmp - pszCmd + 1)) == NULL) {
			for (iCount--; iCount >= 0; iCount--)
				free(ppszCmdA[iCount]);
			free(ppszCmdA);
			free(pPktData);
			return NULL;
		}
		for (pszD = ppszCmdA[iCount], pszS = pszCmd; pszS < pszTmp;) {
			if (*pszS == '\\' && pszS + 1 < pszTmp &&
			    strchr("\" ", pszS[1]) != NULL)
				pszS++;
			*pszD++ = *pszS++;
		}
		*pszD = 0;
		iCount++;
		for (pszCmd = pszTmp; *pszCmd != 0 && *pszCmd != ' '; pszCmd++);
	}
	ppszCmdA[iCount] = NULL;
	free(pPktData);

	return ppszCmdA;
}

static void WmcFreeCmd(char **ppszCmdA) {
	int i;

	for (i = 0; ppszCmdA[i] != NULL; i++)
		free(ppszCmdA[i]);
	free(ppszCmdA);
}

static int WmcHandleCmd(WmcCtx *pCtx, char **ppszCmdA) {
	int i;

	for (i = 0; i < (int) WMCM_COUNTOF(Cmds); i++)
		if (strcmp(ppszCmdA[0], Cmds[i].pszCmd) == 0)
			return (*Cmds[i].pfnCmdProc)(pCtx, ppszCmdA);

	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid command: '%s'\n",
			  ppszCmdA[0]) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}

static int WmcHandleSession(WmcCtx *pCtx) {
	int iError;
	char **ppszCmdA;
	char const *pszWelcome = "WmConsole " WMCONSOLE_VERSION " server ready";

	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s <%s>\n",
			  pszWelcome, pCtx->szRndStr) < 0)
		return -1;
	if ((iError = WmcLoginSession(pCtx)) != 0)
		return iError;
	for (iError = 0; iError >= 0 && iError != WMC_ERR_EXIT &&
	     iError != WMC_ERR_SHUTDOWN;) {
		if ((ppszCmdA = WmcGetCmd(pCtx)) == NULL)
			return -1;
		iError = WmcHandleCmd(pCtx, ppszCmdA);
		WmcFreeCmd(ppszCmdA);
	}

	return iError;
}

static char *WmcGetUserPasswd(WmcCtx *pCtx, char const *pszUser) {
	FILE *pPwdFile;
	char *pszPasswd;
	char szLine[256];

	if ((pPwdFile = fopen("\\Program Files\\WmConsole\\passwd", "r")) == NULL)
		return NULL;
	while (fgets(szLine, sizeof(szLine), pPwdFile) != NULL) {
		WMCM_STRRTRIM(szLine, "\r\n\t ");
		if ((pszPasswd = strchr(szLine, ':')) == NULL)
			continue;
		*pszPasswd++ = '\0';
		if (strcmp(szLine, pszUser) == 0) {
			fclose(pPwdFile);
			return _strdup(pszPasswd);
		}
	}
	fclose(pPwdFile);

	return NULL;
}

static int WmcVerifyLogin(WmcCtx *pCtx, char const *pszUser, char const *pszPwdSha) {
	int i;
	unsigned int uXVal;
	char *pszPasswd;
	sha1_ctx_t ShCtx;
	unsigned char ShaDgst[SHA1_DIGEST_SIZE];

	if (strlen(pszPwdSha) != 2 * sizeof(ShaDgst))
		return -1;
	if ((pszPasswd = WmcGetUserPasswd(pCtx, pszUser)) == NULL)
		return -1;
	sha1_init(&ShCtx);
	sha1_update(&ShCtx, (unsigned char const *) pCtx->szRndStr,
		    strlen(pCtx->szRndStr));
	sha1_update(&ShCtx, (unsigned char const *) ",", 1);
	sha1_update(&ShCtx, (unsigned char const *) pszPasswd, strlen(pszPasswd));
	sha1_final(ShaDgst, &ShCtx);
	free(pszPasswd);
	for (i = 0; i < (int) sizeof(ShaDgst); i++)
		if (sscanf(pszPwdSha + 2 * i, "%2x", &uXVal) != 1 ||
		    uXVal != (unsigned int) ShaDgst[i])
			return -1;

	return 0;
}

static int WmcLoginSession(WmcCtx *pCtx) {
	int iChNbr;
	long lPktSize;
	void *pPktData;
	char *pszUser, *pszPwdSha;

	if (WmcPktRecv(pCtx->pCh, &iChNbr, &pPktData, &lPktSize) < 0)
		return -1;
	pszUser = (char *) pPktData;
	if (WmcPktRecv(pCtx->pCh, &iChNbr, &pPktData, &lPktSize) < 0) {
		free(pszUser);
		return -1;
	}
	pszPwdSha = (char *) pPktData;
	if (WmcVerifyLogin(pCtx, pszUser, pszPwdSha) < 0) {
		WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Bad login for user '%s'\n",
			      pszUser);
		free(pszPwdSha);
		free(pszUser);
		return WMC_ERR_BADLOGIN;
	}
	free(pszPwdSha);
	free(pszUser);

	return WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0);
}

int main(int iArgc, char const **ppszArgs) {
	int iError;
	WmcCtx *pCtx;
	WmcSvcConfig Wcfg;

	if (WmcParseConfig(&Wcfg, iArgc, ppszArgs) < 0)
		return 1;
	if (WmcCtxCreate(&pCtx, &Wcfg) < 0)
		return 2;
	for (;;) {
		if (WmcCtxSessionOpen(pCtx) < 0) {
			WmcCtxClose(pCtx);
			return 3;
		}
		iError = WmcHandleSession(pCtx);
		WmcCtxSessionClose(pCtx);
		if (iError == WMC_ERR_SHUTDOWN)
			break;
		if (iError < 0) {
			WmcCtxClose(pCtx);
			return 4;
		}
	}
	WmcCtxClose(pCtx);

	return 0;
}

