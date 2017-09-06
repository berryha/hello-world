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
#include "infodialog.h"
#include "ui_infodialog.h"
#include <ioaccess.h>
#include "utility.h"

extern unsigned int g_board_info;
#define NET_CONFIG_PATH                     "/usr/ast/uiconf.conf"

#define MAX_COMMAND_LENGHT 256

enum {FIXIP_MODE,DHCP_MODE};

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    init_string();
    ui->devname_label_4->setText(GetBuildDate());
}
void InfoDialog::init_string()
{	
    if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
    {
        ui->devname_label_3->setText("MC5281R");
    }
    else
    {
        ui->devname_label_3->setText("MC5181R");
    }

    FILE *fconf;
    QString macList;
    QString macARM;
    QString macOri;
    char cmdline[MAX_COMMAND_LENGHT];

    //read uiconf.conf to check use DHPC or not
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    if(readfromfile("USE_DHCP",cmdline,NET_CONFIG_PATH))
    {
        int net_mode=atoi(cmdline);
        if(net_mode == DHCP_MODE)
        {
            //read etho0 ip address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            sprintf(cmdline,"ifconfig eth0 | awk '/inet addr/{print substr($2,6)}'");
            fconf=popen(cmdline,"r");
            if(fconf!=NULL)
            {
                memset(cmdline,0,MAX_COMMAND_LENGHT);
                ui->ip_label->setText(tr("No Network!"));
            if(fgets(cmdline,20, fconf) != NULL)
            {
                if(strlen(cmdline)>0)
                {
                cmdline[strlen(cmdline)-1]=0;
                ui->ip_label->setText(QString(cmdline));
                }
            }
                pclose(fconf);
                fconf=0;
            }
            else{
                //syslog
                PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
            }

        }else //use fix ip
        {

            //read fix ip address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("IP_ADDR",cmdline,NET_CONFIG_PATH))
            {
                ui->ip_label->setText(QString(cmdline));
            }
//            //read fix netmask address
//            memset(cmdline,0,MAX_COMMAND_LENGHT);
//            if(readfromfile("NETMASK",cmdline,NET_CONFIG_PATH))
//            {
//                netmaskstr=QString(cmdline);
//                ui->NetMask->setText(QString(cmdline));
//            }
//            //read fix gateway address
//            memset(cmdline,0,MAX_COMMAND_LENGHT);
//            if(readfromfile("GATEWAY",cmdline,NET_CONFIG_PATH))
//            {
//                gatewaystr=QString(cmdline);
//                ui->GateWay->setText(QString(cmdline));
//            }
//            //read DNS1 address
//            memset(cmdline,0,MAX_COMMAND_LENGHT);
//            if(readfromfile("DNS1",cmdline,NET_CONFIG_PATH))
//            {
//                DNS1str=QString(cmdline);
//                ui->DNS1->setText(QString(cmdline));
//            }
//            //read DNS1 address
//            memset(cmdline,0,MAX_COMMAND_LENGHT);
//            if(readfromfile("DNS2",cmdline,NET_CONFIG_PATH))
//            {
//                DNS2str=QString(cmdline);
//                ui->DNS2->setText(QString(cmdline));
//            }
        }
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

int InfoDialog::readfromfile(const char* keyitem,char* value,const char* path)
{
    QString retstr;
    int ret=0;
    int sublen=0;
    FILE *fconf;
    char cmdline[MAX_COMMAND_LENGHT];
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    sublen=strlen(keyitem)+2;
    sprintf(cmdline,"awk '/%s=/ {print substr($0,%d,20)}' %s",keyitem,sublen,path);
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        if(fgets(cmdline,20, fconf) != NULL)
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

void InfoDialog::translateLanguage()
{
    ui->retranslateUi(this);
}

QString InfoDialog::GetBuildDate()
{
    char Date[] = __DATE__;
    char Time[] = __TIME__;
    char mon_s[20] = {0};
    memset(mon_s, 0, 20);
    const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int mon = 0, day = 0, year = 0;
    int hour = 0, min = 0, sec = 0;
    sscanf(Date, "%s %d %d", mon_s, &day, &year);
    mon = (strstr(month_names, mon_s)-month_names)/3 + 1;
    sscanf(Time, "%d:%d:%d", &hour, &min, &sec);
    memset(mon_s, 0, 20);
    sprintf(mon_s, "YMAAP%d%02d%02d%02d%02d", year, mon, day, hour, min);
    QString str = QString(mon_s);
    qDebug()<<str;
    return str;
}

