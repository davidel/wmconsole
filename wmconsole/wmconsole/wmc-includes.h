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


#if !defined(_WMC_INCLUDES_H)
#define _WMC_INCLUDES_H

#include <windows.h>
#include <service.h>
#include <devload.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <bthapi.h>
#include <bthutil.h>
#include <tlhelp32.h>

#if !defined(_HAS_PKFUNCS_H)
/*
 * Definitions imported by the <pkfuncs.h> import file. If you have the Platform
 * Builder, you can add the proper include directory and include <pkfuncs.h> directly.
 */
#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C" BOOL SetStdioPathW(DWORD id, LPCWSTR pwszPath);
extern "C" BOOL KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize,
				LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned);
extern "C" BOOL EnumServices(PBYTE pBuffer, DWORD *pdwServiceEntries, DWORD *pdwBufferLen);
#else
#include <pkfuncs.h>
#endif

#if !defined(_HAS_TOOLHELP_H)
#if !defined(TH32CS_SNAPNOHEAPS)
#define TH32CS_SNAPNOHEAPS 0x40000000
#endif
#else
#include <toolhelp.h>
#endif


#include "rconsole.h"
#include "wmc-macros.h"
#include "wmc-types.h"
#include "wmc-errors.h"
#include "wmc-defines.h"
#include "wmc-pktio.h"
#include "wmc-utils.h"
#include "wmc-regutils.h"
#include "wmc-btutils.h"
#include "wmc-sdprec.h"
#include "wmc-sha1.h"
#include "wmc-context.h"
#include "wmc-cmd-exit.h"
#include "wmc-cmd-shutdown.h"
#include "wmc-cmd-help.h"
#include "wmc-cmd-cd.h"
#include "wmc-cmd-pwd.h"
#include "wmc-cmd-mkdir.h"
#include "wmc-cmd-rmdir.h"
#include "wmc-cmd-ls.h"
#include "wmc-cmd-rm.h"
#include "wmc-cmd-mv.h"
#include "wmc-cmd-cp.h"
#include "wmc-cmd-chmod.h"
#include "wmc-cmd-rmtree.h"
#include "wmc-cmd-reboot.h"
#include "wmc-cmd-find.h"
#include "wmc-cmd-put.h"
#include "wmc-cmd-get.h"
#include "wmc-cmd-ps.h"
#include "wmc-cmd-lsmod.h"
#include "wmc-cmd-wld.h"
#include "wmc-cmd-kill.h"
#include "wmc-cmd-run.h"
#include "wmc-cmd-df.h"
#include "wmc-cmd-mem.h"
#include "wmc-cmd-lssvc.h"
#include "wmc-cmd-lsdev.h"
#include "wmc-cmd-svc.h"
#include "wmc-cmd-rcd.h"
#include "wmc-cmd-rls.h"
#include "wmc-cmd-rpwd.h"
#include "wmc-cmd-rcat.h"
#include "wmc-cmd-rfind.h"
#include "wmc-cmd-rrm.h"
#include "wmc-cmd-rrmtree.h"
#include "wmc-cmd-rmkkey.h"
#include "wmc-cmd-rset.h"
#include "wmc-cmd-rexp.h"
#include "wmc-cmd-rimp.h"
#include "wmc-cmd-rdig.h"


#endif

