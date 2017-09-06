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
#include "dhcp_thread.h"

dhcp_thread::dhcp_thread(QObject *parent) : QObject(parent)
{

}
void dhcp_thread::setcmdline(char* pcmd)
{
    qstrcmd=QString(pcmd);
}
void dhcp_thread::run()
{
    FILE *fconf;
    char cmdline[MAX_COMMAND_LENGHT];

        memset(cmdline,0,MAX_COMMAND_LENGHT);
        //renew ip        
        if(qstrcmd.length()>0)
        {
        	sprintf(cmdline,qstrcmd.toLatin1().data());
        }
        else
        {
        	sprintf(cmdline,"udhcpc -t 3 -T 3 -q -n");
        }
        fconf=popen(cmdline,"r");
        if(fconf!=NULL)
        {
            //get result
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            while(fgets(cmdline,128, fconf) != NULL)
            {
            	
                if(strlen(cmdline)>0)
                {
			    //i=0;
			cmdline[strlen(cmdline)-1]=0;
			if(!strcmp(cmdline,"No lease, failing"))
			{
			    emit changestat(QString("No Network!"));
			    //flag=0;
			    break;
			}
                }
                
            }
         pclose(fconf);
         fconf=0;
        }
        else
        {
           //syslog
          PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }
    emit finished();
}
