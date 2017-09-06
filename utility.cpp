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
#include "utility.h"

Utility::Utility(QObject *parent) : QObject(parent)
{

}
int Utility::check_link(int connection)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];
    char cmdvalue[MAX_COMMAND_LEN];
    if(connection)
    {
		memset(cmdline,0,MAX_COMMAND_LEN);    
		sprintf(cmdline,"cat /sys/class/net/eth0/carrier");
		fd=popen(cmdline,"r");
		if(fd!=NULL)
		{
			memset(cmdline,0,MAX_COMMAND_LEN);
			if(fgets(cmdline,20, fd) != NULL)
			{
				if(strlen(cmdline)>0)
				{
				cmdline[strlen(cmdline)-1]=0;
				ret=atoi(cmdline);
				}
			}
			pclose(fd);
		}
    }
    else//wifi
    {
		memset(cmdline,0,MAX_COMMAND_LEN);	
		sprintf(cmdline,"wpa_cli -i wlan0 status | awk -F\"=\" '/wpa_state/ {print $2}'");
		memset(cmdvalue,0,MAX_COMMAND_LEN);
		if(get_wifi_item(cmdline,cmdvalue))
		{
			if(!strcmp(cmdvalue,"COMPLETED"))
			{
				ret=1;
			}
		}    	
    }
    return ret;
}
int Utility::get_wifi_item(char* cmdstr,char* value)
{
	int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    if(strlen(cmdstr)>0)
    {
		fconf=popen(cmdstr,"r");
		if(fconf!=NULL)
		{
			memset(cmdline,0,MAX_COMMAND_LEN);
			if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
			{
				if(strlen(cmdline)>0)
				{
					cmdline[strlen(cmdline)-1]=0;
					memcpy(value,cmdline,strlen(cmdline));
					ret=1;
				}
			}
			pclose(fconf);
			fconf=0;
		}    	
    }
    else
    {
    	qDebug("command string not found\n");
    }
	return ret;
}
int Utility::isValidMAC(QString mac)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    //read etho0 ip address
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"echo '%s' | egrep '^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$'", mac.toLatin1().data());
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    if(fgets(cmdline,MAX_COMMAND_LEN, fd) != NULL)
	    {
		if(strlen(cmdline)>0)
		{
			ret=1;
		}
	    }
	    pclose(fd);
    }
    else
    {
    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;	
}
int Utility::isValidIP(QString addr)
{
    struct addrinfo naddrinfo, *pinfo =0;
    memset(&naddrinfo, 0, sizeof(naddrinfo));
    naddrinfo.ai_family = AF_UNSPEC;
    naddrinfo.ai_flags = AI_NUMERICHOST;
    int ret = getaddrinfo(addr.toLatin1().data(), 0, &naddrinfo, &pinfo);
    if (ret)
        return -1;
    int result = pinfo->ai_family;
    freeaddrinfo(pinfo);
    return result;
}

int Utility::savetofile(const char* keyitem,char* olddata,char* newdata,const char* path)
{
    FILE *fconf;
    int ret=1;
    int linenum=0;
    char cmdline[MAX_COMMAND_LEN];    
    if(olddata!=NULL)
    {
    	memset(cmdline,0,MAX_COMMAND_LEN);
		sprintf(cmdline,"%s=%s",keyitem,olddata);
		//find old data in which line
		linenum=find_string_infile(cmdline,path);
		if(linenum!=0)
		{
				//delete the old data
				delete_number_line(linenum,path);
		}
    }
    //add new data in tail
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"echo \"%s=%s\" >> %s",keyitem,newdata,path);    
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
    	    pclose(fconf);
    	    ret=1;
    }
    else
    {
        //syslog
        PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }    
    system("sync");
    return ret;
}
int Utility::readfromfile(const char* keyitem,char* value,const char* path)
{
    QString retstr;
    int ret=0;
    int sublen=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(file_exist(path))
    {
	    sublen=strlen(keyitem)+2;
	    sprintf(cmdline,"awk '/^%s=/ {print substr($0,%d,%d)}' %s",keyitem,sublen,MAX_READ_LENGTH,path);
	    fconf=popen(cmdline,"r");
	    if(fconf!=NULL)
	    {
		memset(cmdline,0,MAX_COMMAND_LEN);
		if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
		{
		    if(strlen(cmdline)>0)
		    {
		    cmdline[strlen(cmdline)-1]=0;
		    memcpy(value,cmdline,strlen(cmdline));
		    ret=1;
		    }
		}
		pclose(fconf);
	    }
    }
    return ret;
}
int Utility::insert_head(char* value,const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    if(!file_exist(path) || !file_bytecount(path))//also have to check content length is not zero
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    fconf=fopen(path,"w");
	    fputs(value, fconf);
	    fclose(fconf);
    	    fconf=0;
    }
    else
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    sprintf(cmdline,"echo -n \"%s\" | cat - %s > /tmp/tmp && mv /tmp/tmp %s",value,path,path);
	    fconf=popen(cmdline,"r");
	    if(fconf!=NULL)
	    {
		pclose(fconf);
		fconf=0;
		ret=1;
	    }
    }
    system("sync");
    return ret;		
}
int Utility::file_bytecount(const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"wc -c %s | awk '{print $1}'",path);;
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
    	    //printf("no data in file\n");
    }    
    return ret;		
}
int Utility::file_exist(const char* path)
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
int Utility::read_line_from_file(int linenum,char* odata, const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"awk 'NR==%d' %s",linenum,path);;
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        memset(cmdline,0,MAX_COMMAND_LEN);
        if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {
                cmdline[strlen(cmdline)-1]=0;
                memcpy(odata,cmdline,strlen(cmdline));
                ret=1;
            }
        }
        pclose(fconf);
        fconf=0;
    }
    return ret;
}

int Utility::delete_number_line(int num,const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"sed -i '%dd' %s",num,path);;
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        pclose(fconf);
        fconf=0;
        ret=1;
    }
    system("sync");
    return ret;		
}
int Utility::delete_tail(const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"sed -i '$d' %s",path);;
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        pclose(fconf);
        fconf=0;
        ret=1;
    }
    system("sync");
    return ret;		
}
int Utility::find_string_infile(char* value,const char* path)
{
    int ret=0;
    int linenum=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    fconf=fopen(path,"r");
    if(fconf!=NULL)
    {        
        while(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {
            	    linenum++;
		    cmdline[strlen(cmdline)]=0;
		    if(!memcmp(cmdline,value,strlen(value)))
		    {
		    	ret=linenum;
		    	break;
		    }
            }
            memset(cmdline,0,MAX_COMMAND_LEN);
        }
        pclose(fconf);
        fconf=0;
    }
    return ret;		
}
int Utility::get_line_count(const char* path)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    if(file_exist(path))
    {    
		memset(cmdline,0,MAX_COMMAND_LEN);
		sprintf(cmdline,"wc %s | awk '{print $1}'",path);
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			memset(cmdline,0,MAX_COMMAND_LEN);
			if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
			{
				if(strlen(cmdline)>0)
				{
				cmdline[strlen(cmdline)-1]=0;
				if(strstr (cmdline,"No such file or directory")!=NULL)
				{
					
				}
				else
				{
						ret=atoi(cmdline);
				}
				}
			}
			pclose(fconf);
			fconf=0;
		}
    }
    return ret;	
}
int Utility::readfromenv(const char* keyitem,char* value)
{
    QString retstr;
    int ret=0;
    int sublen=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sublen=strlen(keyitem)+2;
    sprintf(cmdline,"fw_printenv | awk '/^%s=/ {print substr($0,%d,32)}'",keyitem,sublen);
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        memset(cmdline,0,MAX_COMMAND_LEN);
        if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {            	   
               cmdline[strlen(cmdline)-1]=0;
               memcpy(value,cmdline,strlen(cmdline));
               ret=1;
            }
        }
        pclose(fconf);
        fconf=0;
    }
    return ret;	
}
int Utility::writetoenv(const char* keyitem,char* value)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"fw_setenv %s %s",keyitem,value);
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        pclose(fconf);
        fconf=0;
        ret=1;
        system("sync");
    }
    return ret;		
}
int Utility::get_currentIP(char* netif,char* ipbuf)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    //read net interface ip address
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"ifconfig %s | awk '/inet addr/{print substr($2,6)}'",netif);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    //ui->DHCP_IP->setText("No Network!");
	    if(fgets(cmdline,MAX_COMMAND_LEN, fd) != NULL)
	    {
			if(strlen(cmdline)>0)
			{
				cmdline[strlen(cmdline)-1]=0;
				memcpy(ipbuf,cmdline,strlen(cmdline));
				ret=1;
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
int Utility::host_to_ip(char* hostname,char* ipaddr)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"ipquery %s",hostname);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    if(fgets(cmdline,MAX_COMMAND_LEN, fd) != NULL)
	    {
		if(strlen(cmdline)>0)
		{
			cmdline[strlen(cmdline)-1]=0;
			memcpy(ipaddr,cmdline,strlen(cmdline));
			ret=1;
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
int Utility::file_size(const char* filename)
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
int Utility::cp_copyfile(const char* fromfile,const char* tofile)
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
	    system("sync");
    }
    else
    {
    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;	
}
int Utility::append_file(const char* fromfile,const char* tofile)
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
	    system("sync");
    }
    else
    {
    	    //syslog
 	   PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
    }	
    return ret;		
}
int Utility::dd_copyfile(const char* fromfile,const char* tofile,int background)
{
    int ret=0;
    //FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(background)
    {
    	sprintf(cmdline,"dd if=%s of=%s&",fromfile,tofile);
    }
    else
    {
    	sprintf(cmdline,"dd if=%s of=%s",fromfile,tofile);    
    }
    system(cmdline);
    system("sync");
    return ret;		
}
int Utility::detect_cmd_status(char* appname,char* value)
{
    int ret=0;
    FILE *fd;
    char cmdline[MAX_COMMAND_LEN];	
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"kill -USR1 $(pgrep ^%s)",appname);
    fd=popen(cmdline,"r");
    if(fd!=NULL)
    {
	    memset(cmdline,0,MAX_COMMAND_LEN);
	    if(fgets(cmdline,MAX_COMMAND_LEN, fd) != NULL)
	    {
		if(strlen(cmdline)>0)
		{
			cmdline[strlen(cmdline)-1]=0;
			memcpy(value,cmdline,strlen(cmdline));
			ret=1;
		}
	    }    	    
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
int Utility::GetBuildDate(char* value)
{
    unsigned int ret;
    char Date[] = __DATE__;
    char mon_s[20] = {0};
    const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int mon = 0, day = 0, year = 0;

    sscanf(Date, "%s %d %d", mon_s, &day, &year);
    mon = (strstr(month_names, mon_s)-month_names)/3 + 1;
    ret = (mon << 20) | (day<<16) | year;

    sprintf(value, "%d.%02d.%02d", year%2014, mon, day);

    return ret;
}
int Utility::isValidMask(char* netmask)
{
	int ret=0;
	struct in_addr maskaddr;
	unsigned long mask,tmpmask,chkmask;
	do
	{
		if (inet_aton(netmask, &maskaddr) == 0) 
		{
			//PRINT_LOG((LOG_ERR,"Invalid Netmask\n"));
			break;
		}
		mask=ntohl(maskaddr.s_addr);
		tmpmask=~mask;
		chkmask=tmpmask+1;
		if(!(chkmask&tmpmask))
		{
			ret=1;
		}
		else
		{
			//PRINT_LOG((LOG_ERR,"Invalid Netmask\n"));
			break;			
		}
    }while(0);
	return ret;
}
int Utility::get_fwversion(char* fwver)
{
    int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    sprintf(cmdline,"astcmd read fwversion | awk -F= '{print $2}'");
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        memset(cmdline,0,MAX_COMMAND_LEN);
        if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {            	   
               cmdline[strlen(cmdline)-1]=0;
               memcpy(fwver,cmdline,strlen(cmdline));
               ret=1;
            }
        }
        pclose(fconf);
        fconf=0;
    }
    return ret;	
}
int Utility::is_IF_up(char* mif)
{
	int ret=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);    
	//sprintf(cmdline,"ip link show | awk '/%s/ {print $3}'",mif);
	sprintf(cmdline,"ip link show | grep '%s' | grep -c 'UP'",mif);
	fconf=popen(cmdline,"r");
	if(fconf!=NULL)
	{
		memset(cmdline,0,MAX_COMMAND_LEN);
		if(fgets(cmdline,MAX_COMMAND_LEN-1, fconf) != NULL)
		{
			if(strlen(cmdline)>0)
			{
				cmdline[strlen(cmdline)-1]=0;				
				ret=atoi(cmdline);
			}
		}
		pclose(fconf);
	}

	return ret;
}