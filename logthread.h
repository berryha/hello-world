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
#ifndef LOGTHREAD_H
#define LOGTHREAD_H

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
#define MAX_LOG_TEXT_LINE	500
#define LOG_FILE_PATH "/tmp/eventlog/logfile"
#define LOG_FILE_0_PATH "/tmp/eventlog/logfile.0"
#define TMP_LOG_PATH "/tmp/eventlog/eventlog.txt"
#define LOGID_SIZE 10
#define MAX_LINE_SIZE 512
#define PRINT_LOG(fmt) syslog fmt;
class logthread : public QObject
{
    Q_OBJECT
public: 
	int file_exist(const char* path);	
    FILE *logfile;	
    explicit logthread(QObject *parent = 0);	
    int append_file(const char* fromfile,const char* tofile);	
    int cp_copyfile(const char* fromfile,const char* tofile);
    int file_size(const char* filename);
    FILE *tmpfile;
    int filesize;
    QMutex quitkey;
signals:
    void DataChanged(QString logstr);
    void finished();
public slots:
    void run();
};

#endif // LOGTHREAD_H
