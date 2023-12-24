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




typedef struct s_WmcRegExpCtx {
	WmcCtx *pCtx;
	FILE *pFile;
} WmcRegExpCtx;




static wchar_t *WmcRValExportSingle(wchar_t const *pszRPathW);
static wchar_t *WmcRValExport(wchar_t const *pszRPathW);
static int WmcRValFileExportSingle(wchar_t const *pszRPathW, FILE *pFile);
static int WmcRValFileExport(wchar_t const *pszRPathW, FILE *pFile);
static int WmcRKeyFileExport(wchar_t const *pszRPathW, FILE *pFile);
static int WmcCmdExpEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf);




static wchar_t *WmcRValExportSingle(wchar_t const *pszRPathW) {
	wchar_t *pszDup, *pszVName, *pszVExp, *pszExport;
	WmcRValData RVal;

	if (WmcRegGetValueW(pszRPathW, &RVal) < 0)
		return NULL;
	if ((pszDup = WmcRegLongPathW(pszRPathW)) == NULL ||
	    (pszVName = WmcRegValuePos(pszDup)) == NULL ||
	    (pszVExp = WmcRegValExport(&RVal)) == NULL) {
		free(pszDup);
		WmcRegFreeValue(&RVal);
		return NULL;
	}
	*pszVName++ = 0;
	pszExport = WmcSPrintfW(L"[%s]\n\"%s\"=%s\n", pszDup, pszVName, pszVExp);
	free(pszVExp);
	free(pszDup);
	WmcRegFreeValue(&RVal);

	return pszExport;
}

static wchar_t *WmcRValExport(wchar_t const *pszRPathW) {
	wchar_t *pszVExp, *pszExport;
	wchar_t const *pszVName;
	WmcRValData RVal;

	if (WmcRegGetValueW(pszRPathW, &RVal) < 0)
		return NULL;
	if ((pszVName = WmcRegValuePos(pszRPathW)) == NULL ||
	    (pszVExp = WmcRegValExport(&RVal)) == NULL) {
		WmcRegFreeValue(&RVal);
		return NULL;
	}
	pszExport = WmcSPrintfW(L"\"%s\"=%s\n", pszVName + 1, pszVExp);
	free(pszVExp);
	WmcRegFreeValue(&RVal);

	return pszExport;
}

static int WmcRValFileExportSingle(wchar_t const *pszRPathW, FILE *pFile) {
	wchar_t *pszExportW;
	char *pszExport;

	if ((pszExportW = WmcRValExportSingle(pszRPathW)) == NULL)
		return -1;
	if ((pszExport = WmcWide2Byte(pszExportW, -1, 0)) == NULL) {
		free(pszExportW);
		return -1;
	}
	fprintf(pFile, "\n%s", pszExport);
	free(pszExport);
	free(pszExportW);

	return 0;
}

static int WmcRValFileExport(wchar_t const *pszRPathW, FILE *pFile) {
	wchar_t *pszExportW;
	char *pszExport;

	if ((pszExportW = WmcRValExport(pszRPathW)) == NULL)
		return -1;
	if ((pszExport = WmcWide2Byte(pszExportW, -1, 0)) == NULL) {
		free(pszExportW);
		return -1;
	}
	fprintf(pFile, "%s", pszExport);
	free(pszExport);
	free(pszExportW);

	return 0;
}

static int WmcRKeyFileExport(wchar_t const *pszRPathW, FILE *pFile) {
	wchar_t *pszFPathW;
	char *pszFPath;

	if ((pszFPathW = WmcRegLongPathW(pszRPathW)) == NULL)
		return -1;
	pszFPath = WmcWide2Byte(pszFPathW, -1, 0);
	free(pszFPathW);
	fprintf(pFile, "\n[%s]\n", pszFPath);
	free(pszFPath);

	return 0;
}

static int WmcCmdExpEnumCb(void *pPrivate, wchar_t const *pszPathW, WmcKeyInfo const *pKInf) {
	WmcRegExpCtx *pReCtx = (WmcRegExpCtx *) pPrivate;
	int iError;

	if (pKInf->dwType == REG_KEYTYPE)
		iError = WmcRKeyFileExport(pszPathW, pReCtx->pFile);
	else
		iError = WmcRValFileExport(pszPathW, pReCtx->pFile);

	return iError;
}

int WmcCmd_rexp(WmcCtx *pCtx, char **ppszArgsA) {
	int iArgN, iRecurse = 0, iError;
	wchar_t *pszRPathW, *pszPathW;
	FILE *pFile;
	WmcKeyInfo KInf;

	for (iArgN = 1; ppszArgsA[iArgN] != NULL; iArgN++) {
		if (strcmp(ppszArgsA[iArgN], "-R") == 0)
			iRecurse++;
		else
			break;
	}
	if (ppszArgsA[iArgN] == NULL || ppszArgsA[iArgN + 1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argouments\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_SYNTAX;
	}
	if ((pszRPathW = WmcGetPathW(pCtx->pszRegCWD, ppszArgsA[iArgN])) == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[iArgN]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if ((pszPathW = WmcGetPathW(pCtx->pszCWD, ppszArgsA[iArgN + 1])) == NULL) {
		free(pszRPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid path: '%s'\n",
				  ppszArgsA[iArgN + 1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_INVALIDARG;
	}
	if (WmcRegGetPathInfoW(pszRPathW, &KInf) < 0) {
		free(pszPathW);
		free(pszRPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Registry path not found: '%s'\n",
				  ppszArgsA[iArgN]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_NOTFOUND;
	}
	if ((pFile = _wfopen(pszPathW, L"w")) == NULL) {
		free(pszRPathW);
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot create file: '%s'\n",
				  ppszArgsA[iArgN + 1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_FILEOPEN;
	}
	fputs("REGEDIT4\n", pFile);
	if (KInf.dwType != REG_KEYTYPE)
		iError = WmcRValFileExportSingle(pszRPathW, pFile);
	else {
		if ((iError = WmcRKeyFileExport(pszRPathW, pFile)) == 0) {
			WmcRegExpCtx ReCtx;

			WMCM_ZERODATA(ReCtx);
			ReCtx.pCtx = pCtx;
			ReCtx.pFile = pFile;
			iError = WmcRegWalkTreeW(pszRPathW, NULL, iRecurse,
						 WmcCmdExpEnumCb, &ReCtx);
		}
	}
	fclose(pFile);
	free(pszRPathW);
	if (iError < 0) {
		free(pszPathW);
		DeleteFile(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Registry export failed\n") < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return iError;
	}
	free(pszPathW);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return iError;
}

