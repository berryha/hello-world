/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "Chance Tsai" <chance_tsai@aspeedtech.com>
 *   
 * This file is part of "connect_ui".
 *
 * "connect_ui" is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * "connect_ui" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with "connect_ui"; If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#ifndef _WIFINODE_H_
#define _WIFINODE_H_
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#define WIFI_CONNECTED_NODE "/tmp/wificonnected"
typedef struct wifisec
{
	char mode[32];
	char group_cipher[16];
    char pairwise_cipher[16];
    char auth_suites[16];
	struct wifisec* next;
}m_wifisec,*p_wifisec;
typedef struct wifi_scan{
	char macaddr[18];
	char essid[32];
	char protocol[16];
	char mode[16];
	char Frequency[16];
	char channel[4];
	char encryption[4];
	char bit_rates[16];
	p_wifisec extra;
	char quality;
	char signal_level;
	int  status;
	char key_mgmt[8];
	char group[16];
	struct wifi_scan* next;
}m_wifi_scan,*p_wifi_scan;
#endif
