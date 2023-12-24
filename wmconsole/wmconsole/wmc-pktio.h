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


#if !defined(_WMC_PKTIO_H)
#define _WMC_PKTIO_H


int WmcPktSend(WmcChannel const *pCh, int iChNbr, void const *pData, long lSize);
int WmcPktRecv(WmcChannel const *pCh, int *piChNbr, void **ppData, long *plSize);



#endif

