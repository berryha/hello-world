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
#ifndef DHCP_THREAD_H
#define DHCP_THREAD_H
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
#include "logthread.h"
#include "utility.h"
class dhcp_thread : public QObject
{
    Q_OBJECT
public:	
    QString qstrcmd;	
    explicit dhcp_thread(QObject *parent = 0);
    void setcmdline(char* pcmd);
signals:
    void changestat(QString statstr);
    void finished();
public slots:
    void run();
};

#endif // DHCP_THREAD_H
