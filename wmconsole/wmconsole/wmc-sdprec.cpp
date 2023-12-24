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


const int iSdpRecChOff = 0x1a;
const int iSdpRecSize = 0x44;
BYTE const SdpRec[] = {
	0x35, 0x42, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19,
		0x04, 0x09, 0x09, 0x00, 0x04, 0x35, 0x0c, 0x35,
		0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00,
		0x03, 0x08, 0x0c, 0x09, 0x00, 0x06, 0x35, 0x09,
		0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01,
		0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06,
		0x19, 0x04, 0x09, 0x09, 0x01, 0x00, 0x09, 0x01,
		0x00, 0x25, 0x09, 0x57, 0x4d, 0x43, 0x4f, 0x4e,
		0x53, 0x4f, 0x4c, 0x45
};

