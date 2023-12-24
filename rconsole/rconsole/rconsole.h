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

