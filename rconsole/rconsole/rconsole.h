/*
 *  RConsole by Davide Libenzi (Windows Mobile console driver)
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

#if !defined(_RCONSOLE_H)
#define _RCONSOLE_H

#include <winioctl.h>


#define RCON_DEV_PREFIX "RCX"
#define RCON_DEV_PREFIXL L"RCX"

#define RCON_IOCTL_DEVICE 0x77

#define RCON_IOCTL_SHUTDOWN CTL_CODE(RCON_IOCTL_DEVICE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define RCON_IOCTL_SETOWNER CTL_CODE(RCON_IOCTL_DEVICE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)



typedef struct s_RConIoctlImMaster {
	HANDLE hDevice;
	int iOwner;
} RConIoctlImMaster;



#endif

