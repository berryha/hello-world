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
#include "logthread.h"
#include <QtCore>
#define MAX_COMMAND_LEN 256
logthread::logthread(QObject *parent) : QObject(parent)
{

}
void logthread::run()
{
    int fresult;
    QString msg;
    char* ptrmem=NULL;
    QString text; 
    do
    {
    	if(file_exist(LOG_FILE_0_PATH))
    	{
			if(!cp_copyfile(LOG_FILE_0_PATH,TMP_LOG_PATH))
			{
				PRINT_LOG((LOG_ERR,"copy %s to %s fail\n",LOG_FILE_0_PATH,TMP_LOG_PATH));	
			}
        }
        if(file_exist(LOG_FILE_PATH))
        {
			if(!append_file(LOG_FILE_PATH,TMP_LOG_PATH))
			{
				PRINT_LOG((LOG_ERR,"append %s to %s fail\n",LOG_FILE_PATH,TMP_LOG_PATH));	
			}
        }
        filesize=file_size(TMP_LOG_PATH);
	//allocate memory
	ptrmem=(char*)malloc(filesize+1);
	if(ptrmem==NULL)
	{
		PRINT_LOG((LOG_ERR,"malloc error:%s\n",strerror(errno)));
		break;
	}    
	tmpfile = fopen ( TMP_LOG_PATH, "r" );
	if(tmpfile==NULL)
	{
		PRINT_LOG((LOG_ERR,"fopen %s error:%s\n",TMP_LOG_PATH,strerror(errno)));	
		break;    	    
	}	
	memset(ptrmem,0,filesize+1);
	fresult=fread(ptrmem,sizeof(char),filesize,tmpfile);
	if(fresult!=filesize)
	{
		PRINT_LOG((LOG_ERR,"fread %s error:%s\n",LOG_FILE_0_PATH,strerror(errno)));	
		break;	
	}	
	emit DataChanged(QString(ptrmem));

    }while(0);
    
    if(tmpfile!=NULL)
    {
    	    fclose(tmpfile);
    	    tmpfile=0;
    }
    if(ptrmem!=NULL)
    {
    	    free(ptrmem);
    	    ptrmem=NULL;
    }

    emit finished();
}
int logthread::file_size(const char* filename)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"wc %s | awk '{print $3}'",filename);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    if(fgets(cmdline,MAX_COMMAND_LEN, fd) != NULL)
	    {
		if(strlen(cmdline)>0)
		{
			cmdline[strlen(cmdline)]=0;
			ret=atoi(cmdline);
		}
	    }
	    pclose(fd);
	    fd=0;
    }
    else
    {
    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;	
}
int logthread::cp_copyfile(const char* fromfile,const char* tofile)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"cp %s %s",fromfile,tofile);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    ret=1;
	    pclose(fd);
	    fd=0;
    }
    else
    {    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;	
}
int logthread::append_file(const char* fromfile,const char* tofile)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"cat %s >> %s",fromfile,tofile);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    ret=1;
	    pclose(fd);
	    fd=0;
    }
    else
    {
    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;		
}
int logthread::file_exist(const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"[ -f %s ] && echo '1' || echo '0'",path);;
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
    	memset(cmdline,0,MAX_COMMAND_LEN);    
        if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {
            	    cmdline[strlen(cmdline)]=0;
            	    ret=atoi(cmdline);            	    
            }
            
        }    	    
        pclose(fconf);
        fconf=0;
    }
    if(ret==0)
    {
    	    //printf("file not exist\n");
    }
    return ret;		
}
