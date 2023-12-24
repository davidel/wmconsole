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

