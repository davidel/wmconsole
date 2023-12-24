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
#ifndef __TOOLHELP_H__
#define __TOOLHELP_H__

#include "tlhelp32.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THSNAP_RESERVE (1024*1024)

#define TH32CS_SNAPNOHEAPS	0x40000000	// optimization for text shell to not snapshot heaps

typedef struct TH32HEAPENTRY {
	HEAPENTRY32 heapentry;
	struct TH32HEAPENTRY *pNext;
} TH32HEAPENTRY;

typedef struct TH32HEAPLIST {
	HEAPLIST32 heaplist;
	TH32HEAPENTRY *pHeapEntry;
	struct TH32HEAPLIST *pNext;
} TH32HEAPLIST, *PTH32HEAPLIST;	

typedef struct TH32PROC {
	PROCESSENTRY32 procentry;
	TH32HEAPENTRY *pMainHeapEntry;
	struct TH32PROC *pNext;
} TH32PROC;

typedef struct TH32MOD {
	MODULEENTRY32 modentry;
	struct TH32MOD *pNext;
} TH32MOD;

typedef struct TH32THREAD {
	THREADENTRY32 threadentry;
	struct TH32THREAD *pNext;
} TH32THREAD;

typedef struct THSNAP {
	LPBYTE pNextFree;
	LPBYTE pHighCommit;
	LPBYTE pHighReserve;
	TH32PROC *pProc;
	TH32MOD *pMod;
	TH32THREAD *pThread;
	TH32HEAPLIST *pHeap;

    /* Keep track of the location of snapshot data. */
    TH32HEAPENTRY *pCurHeapEntry;
    TH32HEAPLIST *pCurHeapList;
} THSNAP;

#ifdef __cplusplus
}
#endif

#endif

