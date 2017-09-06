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
#include "cmdthread.h"

CMDThread::CMDThread(QObject *parent) : QObject(parent)
{

}
void CMDThread::setcmdline(char *pcmd)
{
    qstrcmd=QString(pcmd);
}
void CMDThread::run()
{
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    fd=popen(qstrcmd.toLatin1().data(),"r");
    if(fd!=NULL)
    {	
        memset(cmdline,0,MAX_COMMAND_LEN);
        while(fgets(cmdline,MAX_COMMAND_LEN-1, fd) != NULL)
        {
            if(strlen(cmdline)>0)
            {
               cmdline[strlen(cmdline)-1]=0;
               emit changestat(QString(cmdline));
            }
        }
        pclose(fd);
        fd=0;
    }
    emit finished();
}
