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
#ifndef UTILITY_H
#define UTILITY_H
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <dirent.h>
#include <QThread>
#include <QMutex>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include "logthread.h"
#include "cmdthread.h"
#define WIFI_NET_PATH       "usr/ast/wifi_net.conf"
#define UI_STRING_PATH		"usr/ast/ui_string.conf"
#define SETTING_PATH            "/usr/ast/setting.conf"
#define SYSTEM_CONFIG_PATH "/usr/ast/system.conf"
#define ACCOUNT_CONFIG_PATH "/usr/ast/account.conf"

#define MAX_SAVE_PATH_LENGTH 256
#define SAVE_ROOT_PATH "/media/"
#define MAX_COMMAND_LEN 256
#define MAX_READ_LENGTH 128
#define DLGCTRL_GENERAL_SETTINGS	0x0001
#define DLGCTRL_ADVANCE_LOGIN	 	0x0002
#define DLGCTRL_ADVANCE_SETTINGS 	0x0004
//#define DLGCTRL_SYSTEM_LOG			0x0008
#define DLGCTRL_TOOLS				0x0010
#define DLGCTRL_SYSTEM_INFO	 		0x0020
#define DLGCTRL_WIFI_SETTINGS	 	0x0040
#define WAIT_CONF_PATH  "/usr/ast/wait.conf"
#define MESSAGE_INFO_TITLE 	"Information"
#define MESSAGE_WARNING_TITLE	"Warning"
#define MAX_COMMAND_LENGHT 256
#define userinfo_path               "/usr/ast/userinfo.conf"
#define password_path               "/usr/ast/password.conf"
#define setting_path                "/usr/ast/setting.conf"
#define usb_upgrade_path            "/usr/ast/usb_upgrade.conf"



class Utility : public QObject
{
    Q_OBJECT
public:
	int get_wifi_item(char* cmdstr,char* value);
	int is_IF_up(char* mif);
    int GetBuildDate(char* value);
    int detect_cmd_status(char* appname,char* value);	
    int dd_copyfile(const char* fromfile,const char* tofile,int background);	
    int append_file(const char* fromfile,const char* tofile);	
    int cp_copyfile(const char* fromfile,const char* tofile);
    int file_size(const char* filename);
    int host_to_ip(char* hostname,char* ipaddr);
    int read_line_from_file(int linenum,char* odata, const char* path);
    int file_bytecount(const char* path);	
    int file_exist(const char* path);	
    int find_string_infile(char* value,const char* path);
    int insert_head(char* value,const char* path);
    int delete_number_line(int num,const char* path);    
    int delete_tail(const char* path);
    int get_line_count(const char* path);
    int readfromenv(const char* keyitem,char* value);
    int isValidMAC(QString mac);
    int get_currentIP(char* netif,char* ipbuf);
    int check_link(int connection);
    int isValidIP(QString addr);
    int savetofile(const char* keyitem,char* olddata,char* newdata,const char* path);
    int readfromfile(const char* keyitem,char* value,const char* path);
    int writetoenv(const char* keyitem,char* value);
    int isValidMask(char* netmask);
    int get_fwversion(char* fwver);
    explicit Utility(QObject *parent = 0);

signals:

public slots:
};

#endif // UTILITY_H
/*
QPushButton{border-image: url(:/images/images/friendmoe/system_01.png);
}
QPushButton:pressed{border-image: url(:/images/images/friendmoe/system_02.png);
}
QPushButton:focus{border-image:
url(:/images/images/friendmoe/system_03.png);
}

*/
