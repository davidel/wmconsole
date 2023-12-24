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


#if !defined(_WMC_MACROS_H)
#define _WMC_MACROS_H


#define WMCM_ZERODATA(d) memset(&(d), 0, sizeof(d))
#define WMCM_COUNTOF(a) (sizeof(a) / sizeof(a[0]))
#define WMCM_SAMECHAR(i, a, b) ((i) == 0 ? (tolower(a) == tolower(b)): ((a) == (b)))
#define WMCM_SAMECHARW(i, a, b) ((i) == 0 ? (towlower(a) == towlower(b)): ((a) == (b)))
#define WMCM_SETDBUF(b, p, n) do { (b)->pData = (char *) (p); (b)->lSize = (long) (n); } while (0)
#define WMCM_CH_READ(c, b, n) (*(c)->pfnRead)((c)->pPrivate, b, n)
#define WMCM_CH_WRITE(c, b, n) (*(c)->pfnWrite)((c)->pPrivate, b, n)
#define WMCM_GET_LE16(v, p) do { \
	unsigned char const *__p = (unsigned char const *) p; \
	(v) = (((unsigned long) __p[1]) << 8) | ((unsigned long) __p[0]); \
} while (0)
#define WMCM_GET_LE32(v, p) do { \
	unsigned char const *__p = (unsigned char const *) p; \
	(v) = (((unsigned long) __p[3]) << 24) | (((unsigned long) __p[2]) << 16) | \
		(((unsigned long) __p[1]) << 8) | ((unsigned long) __p[0]); \
} while (0)
#define WMCM_PUT_LE16(v, p) do { \
	unsigned char *__p = (unsigned char *) p; \
	*__p++ = (unsigned char) (v); \
	*__p = (unsigned char) ((v) >> 8); \
} while (0)
#define WMCM_PUT_LE32(v, p) do { \
	unsigned char *__p = (unsigned char *) p; \
	*__p++ = (unsigned char) (v); \
	*__p++ = (unsigned char) ((v) >> 8); \
	*__p++ = (unsigned char) ((v) >> 16); \
	*__p = (unsigned char) ((v) >> 24); \
} while (0)
#define WMCM_VSPRINTF(r, a, f) do { \
	int __csize = 256, __psize; \
	va_list __args; \
	for (;;) { \
		if (!(r = (char *) malloc(__csize))) \
			break; \
		__args = a; \
		if ((__psize = _vsnprintf(r, __csize - 1, f, __args)) >= 0 && \
			    __psize < __csize) { \
			va_end(__args); \
			break; \
		} \
		va_end(__args); \
		if (__psize > 0) \
			__csize = (4 * __psize) / 3 + 2; \
		else \
			__csize *= 2; \
		free(r); \
	} \
} while (0)
#define WMCM_VSPRINTFW(r, a, f) do { \
	int __csize = 256, __psize; \
	va_list __args; \
	for (;;) { \
		if (!(r = (wchar_t *) malloc(__csize * sizeof(wchar_t)))) \
			break; \
		__args = a; \
		if ((__psize = _vsnwprintf(r, __csize - 1, f, __args)) >= 0 && \
			    __psize < __csize) { \
			va_end(__args); \
			break; \
		} \
		va_end(__args); \
		if (__psize > 0) \
			__csize = (4 * __psize) / 3 + 2; \
		else \
			__csize *= 2; \
		free(r); \
	} \
} while (0)
#define WMCM_DROPLAST(s, c) do { \
	int __len = strlen(s); \
	if (__len > 0 && (s)[__len - 1] == (c)) \
		(s)[__len - 1] = '\0'; \
} while (0)
#define WMCM_MISSAPPEND(s, c) do { \
	int __len = strlen(s); \
	if (__len == 0 || (s)[__len - 1] != (c)) \
		(s)[__len] = (c), (s)[__len + 1] = '\0'; \
} while (0)
#define WMCM_DROPLASTW(s, c) do { \
	int __len = wcslen(s); \
	if (__len > 0 && (s)[__len - 1] == (wchar_t) (c)) \
		(s)[__len - 1] = 0; \
} while (0)
#define WMCM_MISSAPPENDW(s, c) do { \
	int __len = wcslen(s); \
	if (__len == 0 || (s)[__len - 1] != (wchar_t) (c)) \
		(s)[__len] = (c), (s)[__len + 1] = 0; \
} while (0)
#define WMCM_ZMEMCPY(d, s, n) do { memcpy(d, s, n); (d)[n] = '\0'; } while (0)
#define WMCM_ZMEMCPYW(d, s, n) do { memcpy(d, s, (n) * sizeof(wchar_t)); (d)[n] = '\0'; } while (0)
#define WMCM_STRRTRIM(s, t) do { \
	int __len; \
	for (__len = strlen(s); __len > 0 && strchr(t, (s)[__len - 1]) != NULL; __len--); \
	(s)[__len] = '\0'; \
} while (0)
#define WMCM_STRRTRIMW(s, t) do { \
	int __len; \
	for (__len = wcslen(s); __len > 0 && wcschr(t, (s)[__len - 1]) != NULL; __len--); \
	(s)[__len] = 0; \
} while (0)


#endif

