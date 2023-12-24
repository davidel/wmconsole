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





static int WmcNextToken(FILE *pFile, char const *pszSkip);
static int WmcReadToken(FILE *pFile, char const *pszNot, char const *pszSet,
			char *pszTok, int iSize);
static void *WmcReadRegBinary(FILE *pFile, DWORD *pdwSize);



static char const szHexChars[] = {
	"0123456789abcdefABCDEF"
};
static char const szNameChars[] = {
	"qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM0123456789_"
};




static int WmcNextToken(FILE *pFile, char const *pszSkip) {
	int c;

	while ((c = fgetc(pFile)) != EOF)
		if (pszSkip == NULL || strchr(pszSkip, c) == NULL)
			break;

	return c;
}

static int WmcReadToken(FILE *pFile, char const *pszNot, char const *pszSet,
			char *pszTok, int iSize) {
	int i, c = EOF;

	for (i = 0; i < iSize; i++) {
		if ((c = fgetc(pFile)) == EOF)
			break;
		if (pszNot != NULL && strchr(pszNot, c) != NULL)
			break;
		if (pszSet != NULL && strchr(pszSet, c) == NULL)
			break;
		pszTok[i] = (char) c;
	}
	if (i == iSize)
		return -1;
	pszTok[i] = '\0';
	if (c != EOF)
		ungetc(c, pFile);

	return i;
}

static void *WmcReadRegBinary(FILE *pFile, DWORD *pdwSize) {
	int c;
	unsigned int uHVal;
	DWORD dwAlloc, dwSize;
	unsigned char *pData, *pNData;
	char szHex[4];

	WMCM_ZERODATA(szHex);
	for (pData = NULL, dwSize = dwAlloc = 0;;) {
		if (dwSize == dwAlloc) {
			dwAlloc = 2 * dwAlloc + 256;
			if ((pNData = (unsigned char *)
			     realloc(pData, dwAlloc)) == NULL) {
				free(pData);
				return NULL;
			}
			pData = pNData;
		}
		if (dwSize > 0 && (c = WmcNextToken(pFile, " \t")) != ',') {
			free(pData);
			return NULL;
		}
		if ((c = WmcNextToken(pFile, " \t")) == '\\')
			c = WmcNextToken(pFile, " \t\r\n");
		if (c == EOF || c == '\n')
			break;
		szHex[0] = (char) c;
		szHex[1] = (char) (c = WmcNextToken(pFile, NULL));
		if (c == EOF || sscanf(szHex, "%2x", &uHVal) != 1) {
			free(pData);
			return NULL;
		}
		pData[dwSize++] = (unsigned char) uHVal;
	}
	*pdwSize = dwSize;

	return pData;
}

int WmcCmd_rimp(WmcCtx *pCtx, char **ppszArgsA) {
	int iTok, iError;
	DWORD dwType, dwSize, dwError;
	LONG lResult;
	HKEY hKey = NULL;
	void *pData;
	wchar_t *pszPathW;
	FILE *pFile;
	char szBuf[256], szType[32], szString[1024];

	if (ppszArgsA[1] == NULL) {
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Missing argoument\n") < 0 ||
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
	if ((pFile = _wfopen(pszPathW, L"r")) == NULL) {
		free(pszPathW);
		if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Cannot open file: '%s'\n",
				  ppszArgsA[1]) < 0 ||
		    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
			return -1;
		return WMC_ERR_FILEOPEN;
	}
	free(pszPathW);
	if (fgets(szBuf, sizeof(szBuf), pFile) == NULL ||
	    memcmp(szBuf, "REGEDIT4\n", 9) != 0) {
		fclose(pFile);
		goto RegFmtError;
	}
	for (;;) {
		if ((iTok = WmcNextToken(pFile, " \t\r\n")) == EOF)
			break;
		if (iTok == ';') {
			while ((iTok = fgetc(pFile)) != EOF && iTok != '\n');
			if (iTok == EOF)
				break;
		} else if (iTok == '[') {
			wchar_t *pszRegPathW, *pszRPathW;

			if (WmcReadToken(pFile, "]", NULL, szBuf, sizeof(szBuf)) < 0 ||
			    WmcNextToken(pFile, NULL) != ']')
				goto RegFmtError;
			if ((pszRegPathW = WmcByte2Wide(szBuf, -1, 0)) == NULL ||
			    (pszRPathW = WmcRegShortPathW(pszRegPathW)) == NULL) {
				free(pszRegPathW);
				goto RegFmtError;
			}
			free(pszRegPathW);
			if (hKey != NULL)
				RegCloseKey(hKey);
			iError = WmcRegMkPathW(pszRPathW, 1, &hKey);
			free(pszRPathW);
			if (iError < 0) {
				fclose(pFile);
				if (WmcChanOSErrorReport(pCtx->pCh, WMC_CHAN_CTRL,
							 "Unable to create key: '%s'\n",
							 szBuf) < 0)
					return -1;
				return WMC_ERR_REGKEYCREATE;
			}
		} else if (iTok == '"') {
			wchar_t *pszNameW;

			if (WmcReadToken(pFile, NULL, szNameChars, szBuf, sizeof(szBuf)) < 0 ||
			    WmcNextToken(pFile, NULL) != '"' ||
			    WmcNextToken(pFile, " \t") != '=' ||
			    (iTok = WmcNextToken(pFile, " \t")) == EOF)
				goto RegFmtError;
			if (iTok == '"') {
				if (WmcReadToken(pFile, "\"", NULL, szString,
						 sizeof(szString)) < 0 ||
				    WmcNextToken(pFile, NULL) != '"')
					goto RegFmtError;
				dwType = REG_SZ;
				dwSize = (strlen(szString) + 1) * sizeof(wchar_t);
				pData = WmcByte2Wide(szString, -1, 0);
			} else {
				szType[0] = (char) iTok;
				if (WmcReadToken(pFile, ":", NULL, szType + 1,
						 sizeof(szType) - 1) < 0 ||
				    WmcNextToken(pFile, NULL) != ':')
					goto RegFmtError;
				if (strcmp(szType, "dword") == 0)
					dwType = REG_DWORD;
				else if (strcmp(szType, "hex") == 0)
					dwType = REG_BINARY;
				else if (strcmp(szType, "hex(7)") == 0)
					dwType = REG_MULTI_SZ;
				else if (strcmp(szType, "hex(2)") == 0)
					dwType = REG_EXPAND_SZ;
				else
					goto RegFmtError;
				if (dwType == REG_DWORD) {
					if (WmcReadToken(pFile, NULL, szHexChars, szString,
							 sizeof(szString)) < 0 ||
					    (pData = malloc(sizeof(DWORD))) == NULL ||
					    sscanf(szString, "%x", pData) != 1)
						goto RegFmtError;
					dwSize = 4;
				} else
					pData = WmcReadRegBinary(pFile, &dwSize);
			}
			if (pData == NULL)
				goto RegFmtError;
			pszNameW = WmcByte2Wide(szBuf, -1, 0);
			lResult = RegSetValueEx(hKey, pszNameW, 0, dwType, (LPBYTE) pData, dwSize);
			dwError = GetLastError();
			free(pszNameW);
			free(pData);
			if (lResult != ERROR_SUCCESS) {
				fclose(pFile);
				if (hKey != NULL)
					RegCloseKey(hKey);
				if (WmcChanOSErrorReportE(pCtx->pCh, WMC_CHAN_CTRL, dwError,
							  "Unable to set value: '%s'\n",
							  szBuf) < 0)
					return -1;
				return WMC_ERR_REGVALUESET;
			}
		}
	}
	fclose(pFile);
	if (hKey != NULL)
		RegCloseKey(hKey);
	if (WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;

	RegFmtError:
	fclose(pFile);
	if (hKey != NULL)
		RegCloseKey(hKey);
	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "Invalid registry file: '%s'\n",
			  ppszArgsA[1]) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;
	return WMC_ERR_CORRUPT;
}

