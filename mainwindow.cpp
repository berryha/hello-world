/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "IC Yang" <ic_yang@aspeedtech.com>
 *         "Chance Tsai" <chance_tsai@aspeedtech.com>
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
#include <QKeyEvent>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <QDateTime>

#include <netdb.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "userconfig.h"

unsigned int g_run_once = 0;

static QString  g_mode[k_mode_num] =
{
    "1920x1080",
    "1600x1200",
    "1600x900",
    "1440x900",    
    "1366x768",
    "1360x768",
    "1280x1024",
    "1280x720",
    "1024x768",
    "800x600"
};

static int  g_mode_w[k_mode_num] =
{
    1920,
    1600,
    1600,
    1440,
    1366,
    1360,
    1280,
    1280,
    1024,
    800
};

static int  g_mode_h[k_mode_num] =
{
    1080,
    1200,
    900,
    900,
    768,
    768,
    1024,
    720,
    768,
    600
};


#define  MODE_FOUND 	0
#define  MODE_OVER 		1
#define  MODE_NOCON		2
#define  MODE_NEWM 		3
#define  MODE_CHANGE 	4

#define CapsLock_ON    (1<<2)
#define NumLock_ON     (1<<1)
#define ScroLock_ON     (1<<0)
static QSettings userinfo(userinfo_path,QSettings::IniFormat);
IO_ACCESS_DATA  io_CRT;
int g_rfxfd = 0;
void* g_pItemStart = 0;
char* g_pFlashData = 0;
char* g_pWallPater = 0;
unsigned int g_board_info = 0;
int g_net_broken = 0;

VESA_MODE AST_ModeTable[] = {
  {1344, 806,  1024, 768,  60, 136, 160, 6, 29, 0, 0, 0, 0,	AST3200_VCLK65,	    HOR_NEGATIVE, VER_NEGATIVE,  -1, 7},
  {1792, 798,  1360,  768, 60, 112, 256, 3, 24, 0, 0, 0, 0, AST3200_VCLK85_8,   HOR_POSITIVE, VER_POSITIVE, -1, 0}  
};

QString g_version_info = "FW Rev  ";
char    g_version_info_str[32] = {0};
unsigned int g_Build_FW_Date = 0;
const char k_flahwp[] = "/flashwp.png";
const char k_wallpaper[] = "/p0_140_238.png";   
const int  k_wallpaper_size = 0x1000;//121581;
const char k_wallpath[] ="/ast";

#define GET_ITEM_DATA(pitem)          (g_pFlashData + pitem->dataoffset)

int getaddrfamily(const char *addr)
{
    struct addrinfo naddrinfo, *pinfo =0;
    memset(&naddrinfo, 0, sizeof(naddrinfo));
    naddrinfo.ai_family = AF_UNSPEC;
    naddrinfo.ai_flags = AI_NUMERICHOST;
    int ret = getaddrinfo(addr, 0, &naddrinfo, &pinfo);
    if (ret)
    {
        return -1;
    }
    int result = pinfo->ai_family;
    freeaddrinfo(pinfo);
    return result;
}

unsigned int GetBuildDate()
{
    unsigned int ret;
    char Date[] = __DATE__;

    char mon_s[20] = {0};
    const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int mon = 0, day = 0, year = 0;
    sscanf(Date, "%s %d %d", mon_s, &day, &year);
    mon = (strstr(month_names, mon_s)-month_names)/3 + 1;
    ret = (mon << 20) | (day<<16) | year;
    sprintf(g_version_info_str, "FW Rev  %d.%02d.%02d", year%2014, mon, day);

    return ret;
}

void MainWindow::getBoardInfo(unsigned int* pbord)
{
    ioctl(m_rlefd, IOCTL_AST_GET_BOARD_INFO, pbord);        
}

int MainWindow::checkCRTstatus()
{
    int statechange = 0;
    int crtidx = m_crtidx;
    int bMM = m_multiMonitor;
    static int s_check_cnt = 0;
    
    getBoardInfo(&g_board_info);
    
	if((g_board_info & (1<<SCRACH_CRT_A_ON_BIT)) && (g_board_info & (1<<SCRACH_CRT_B_ON_BIT)))
	{
		bMM = 1;
	}    
	else if(g_board_info & (1<<SCRACH_CRT_A_ON_BIT))
	{
		crtidx = CRT_A;
		bMM = 0;
	}
	else if(g_board_info & (1<<SCRACH_CRT_B_ON_BIT))
	{
		crtidx = CRT_B;
		bMM = 0;
	}
    if(crtidx != m_crtidx || bMM != m_multiMonitor || 0 == s_check_cnt)
	{
		statechange = 1;
		m_crtidx = crtidx;
		m_multiMonitor = bMM;
    }
    s_check_cnt++;
    return statechange;
}

int MainWindow::checkNETbroken()
{
    int ret = 0;
    FILE* eth0f = 0;
    char mystr[4] = {0};
    char* ptmp;

    eth0f = fopen("/sys/class/net/eth0/carrier", "rt");
    if(eth0f)
    {
        ptmp = fgets(mystr, 4, eth0f);
        if(0 == ptmp || '0' == mystr[0])
        {
            g_net_broken = 1;
            ret = 1;
        }
        fclose(eth0f);
        eth0f=0;
    }
    return ret;
}

void MainWindow::timer_timeout()
{
    int netbk = 0;
    int crt_c;
    char tmpbuf[CMDMAX];
    char cmdline[MAX_COMMAND_LENGTH];
    QMessageBox timermsg;
//	crt_c = checkCRTstatus();
//    if(crt_c|| (!g_run_once))
//	{
//        g_run_once = 1;
//		printf("ui:CRT_%X connected\n", crt_c);
////		TurnOnCRT(m_crtidx);
////		if(m_multiMonitor)
////			TurnOnCRT(!m_crtidx);
////        TurnOnCRT(0);
//        TurnOnCRT(1);
//	}
    if(m_bUseDHCP)
    {
        netbk = checkNETbroken();
        if(g_net_broken)    //ever broken
        {
            if(!netbk)      //now connected
            {
				//timermsg = new QMessageBox;
				timermsg.setParent(this);   	
				timermsg.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);            	
                g_net_broken = 0; 
        		memset(tmpbuf,0,CMDMAX);
        		if(pUtility->readfromfile("MESSAGE_INFO_TITLE",tmpbuf,UI_STRING_PATH))
        		{
        			timermsg.setWindowTitle(QString(tmpbuf));
        		}
        		else
        		{
                    timermsg.setWindowTitle(tr("Information"));
        		}               
                timermsg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                timermsg.setDefaultButton(QMessageBox::Yes);       	
                timermsg.setText(tr("Cable re-connect, do you want to renew IP ?"));
                if(timermsg.exec()==QMessageBox::Yes)    
                {
                    waitwindow = new WaitingDialog(this);
                    waitwindow->setFixedSize(370,50);	
                    waitwindow->setWindowTitle(tr("DHCP Renewing..."));
                    waitwindow->setParent(this);
                    waitwindow->setStyleSheet("background-color:Pale gray;");
                    waitwindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
					//20160225 modify for dhcp renew
					memset(cmdline,0,MAX_COMMAND_LENGTH);
					sprintf(cmdline,"udhcpc -i eth0 -t 3 -T 3 -q -n");
					waitwindow->setcmdline(cmdline);                    
                    connect(waitwindow, SIGNAL(dhcprenewdone()),SLOT(onDHCPdone()));
                    waitwindow->exec();
                }
                //free(timermsg);
            }
            else
            {
 				memset(cmdline,0,MAX_COMMAND_LENGTH);
				sprintf(cmdline,"eth0");           	
            	if(pUtility->is_IF_up(cmdline)==1)
            	{
            		emit NetInvalid();
            	}
            }
        }
    }
}

void MainWindow::TurnOnCRT(int crt_index)
{
//    unsigned int tw = 0;
//    unsigned int th = 0;
//    unsigned int bflip = 0;
//    CRT_MODE     CRTMode;
    
//    io_CRT.Address = 0x1e6e6000 + (crt_index<<8) + 0x70;
//    ioctl (m_rlefd, IOCTL_IO_READ, &io_CRT);
//    tw = (io_CRT.Data >> 16) + 1;

//    io_CRT.Address = 0x1e6e6000 + (crt_index<<8) + 0x78;
//    ioctl (m_rlefd, IOCTL_IO_READ, &io_CRT);
//    th = (io_CRT.Data >> 16) + 1;

//    io_CRT.Address = 0x1e6e6000 + (crt_index<<8) + 0x68;
//    ioctl (m_rlefd, IOCTL_IO_READ, &io_CRT);
//    bflip = io_CRT.Data & (1<<6);

//    CRTMode.crtidx = crt_index;
//    CRTMode.format = XRGB8888_FORMAT;
//    memcpy(&CRTMode.mode, &AST_ModeTable[0], sizeof(VESA_MODE));
    
//    ioctl(m_rlefd, IOCTL_IO_ENABLE_CRT_2, &CRTMode);
//    io_CRT.Type = XRGB8888_FORMAT;
//    io_CRT.Data = 1024;
//    io_CRT.Value = 768;
//    io_CRT.Address = crt_index;	//define in ioaccess.h
//    ioctl(m_rlefd, IOCTL_ENABLE_FB, &io_CRT);


    //170420 latest code from taiwan

    io_CRT.Data = 0;                //x
    io_CRT.Value = 0;               //y
    io_CRT.ExtData = 1024;          //width
    io_CRT.ExtValue = 768;          //height
    io_CRT.Address = crt_index;     //monitor_idx
    ioctl(m_rlefd, IOCTL_MONITOR_POSITION, &io_CRT);

    io_CRT.Type = XRGB8888_FORMAT | FB_DISP;
    io_CRT.Data = 1024;
    io_CRT.Value = 768;
    io_CRT.Address = crt_index;
    ioctl(m_rlefd, IOCTL_IO_ENABLE_CRT, &io_CRT);

}

int MainWindow::ParseConfigLine(char* pstr)
{
    int     ret = 0;
    int     i, j;
    char*   pname = 0;
    char*   pdata = 0;
    
    if('\n' == pstr[0])
    {
        ret = 2;
        return ret;
    }
    
    for(i=0; i<CMDMAX; i++)
    {
        if(CMDMAX == i)
        {
            ret = 1;
            return ret;
        }
        
        if('=' == pstr[i])
        {
            pname = pstr;
            pstr[i] = 0;

            if('\n' == pstr[i+1])
            {
                ret = 0;
                printf("%s empty\n", pname);
                return ret;
            }
            
            for(j=i+1; j<i+8; j++)
            {
                if(' ' != pstr[j])
                    break;
            }    
            pdata = &pstr[j];            
            break;
        }
    }
    
    for(i=0; i<CMDMAX; i++)
    {
        if('\n' == pdata[i])
        {
            pdata[i] = 0;
            break;
        }        
    }
  
    if(0 == strcmp("RESERVED", pname))
    {
        //sscanf( );
    }
    else if (0 == strcmp("USE_DHCP", pname))
    {
        sscanf(pdata, "%x", &m_bUseDHCP);
    }
    else if (0 == strcmp("IP_ADDR", pname))
    {
        sprintf(m_ip_addr, "%s", pdata);
    }    
    else if (0 == strcmp("NETMASK", pname))
    {
        sprintf(m_NetMask, "%s", pdata);
    }
    else if (0 == strcmp("GATEWAY", pname))
    {
        sprintf(m_Gateway, "%s", pdata);
    }    
    else if (0 == strcmp("BOARD_VER", pname))
    {
        sscanf(pdata, "%x", &g_board_info);        
    }  
    else if (0 == strcmp("DATA_VER", pname))
    {
        sprintf(m_data_ver, "%s", pdata);
    }
    else if (0 == strcmp("CRT_ON_IDX", pname))
    {
        sscanf(pdata, "%x", &m_crtidx);
    }
    else if (0 == strcmp("AUTO_MODE_DETECT", pname))
    {
        sscanf(pdata, "%x", &m_bPreferMode);
    }
    else if (0 == strcmp("WALLPAPER", pname))
    {
        sprintf(m_readwall, "%s", pdata);
    }
    else
    {
        ret = 1;    
    }
    return ret;
}
void MainWindow::initial_main()
{
    FILE* rdpf = 0;
    char prestr[CMDMAX] = {0};
    char cmdline[MAX_COMMAND_LENGTH];	
    QString qUsername;
    //system("io 1 1e6e6060 b0030101"); 
    //memset(&g_ConfigData, 0, sizeof(USER_CONFIG_DATA));    
    //showFullScreen();
   // update();

    //crt_timeout();    //delete 20170802 by xby
    
    m_kb_fd = 0;
    m_FWUpdate = 0;
    m_bDataFromFlash = 0;
    m_bPreferMode = 1;
    m_bUseDHCP = 1;
    m_crtidx = CRT_A;
    m_backgroundsize = 0;
    m_frame_ack = 0; 
    m_multiMonitor = 0;
    memset(m_cmdline, 0, CMDMAX);

    g_Build_FW_Date = GetBuildDate();

    //m_rlefd = open("/dev/access", O_RDWR|O_ASYNC);
    mainmsg = new QMessageBox;
    mainmsg->setParent(this);   	
    mainmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    //read and parse hostlist for host combobox
    pUtility = new Utility;
    m_remember_me=read_rememeber_me();
    parse_vhost();
    memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(pUtility->readfromfile("SETTINGS_TITLE",cmdline,UI_STRING_PATH))
	{
		ui->actionSettings->setToolTip(QString(cmdline));
	}
	else
	{
		ui->actionSettings->setToolTip("General Settings");
	} 
	
    //add manager ip for hwver 4
    memset(cmdline,0,MAX_COMMAND_LENGTH);    
    if(pUtility->readfromenv("hwver",cmdline))
    {
    	m_hwver = atoi(cmdline);
    } 	
	 
    memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(pUtility->readfromfile("ADVANCE_TITLE",cmdline,UI_STRING_PATH))
	{
		ui->actionAdvance_Settings->setToolTip(QString(cmdline));
	}
	else
	{
		ui->actionAdvance_Settings->setToolTip("Administrative Settings");
	}  
    memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(pUtility->readfromfile("TOOL_TITLE",cmdline,UI_STRING_PATH))
	{
		ui->actionTools->setToolTip(QString(cmdline));
	}
	else
	{
		ui->actionTools->setToolTip("Tools");
	} 
    memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(pUtility->readfromfile("SYSINFO_TITLE",cmdline,UI_STRING_PATH))
	{
		ui->actionSystem_Information->setToolTip(QString(cmdline));
	}
	else
	{
		ui->actionSystem_Information->setToolTip("System Information");
	} 
    memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(pUtility->readfromfile("MAINWINDOW_POWER",cmdline,UI_STRING_PATH))
	{
		ui->actionShut_down->setToolTip(QString(cmdline));
	}
	else
	{
		ui->actionShut_down->setToolTip("Shut Down");
	} 		   
    
    combo_flag=0;
    dislog_ctrl=0;
    qApp->installEventFilter(this);
    //read DHCP from file
    memset(cmdline,0,MAX_COMMAND_LENGTH);
    if(pUtility->readfromfile("USE_DHCP",cmdline,NET_CONFIG_PATH))
    {
        m_bUseDHCP=atoi(cmdline);
    }         
    combo_flag=1;

    accountflag=1;
    memset(cmdline,0,MAX_COMMAND_LENGTH);    
    if(pUtility->readfromenv("hwver",cmdline))
    {
    	if(atoi(cmdline)==4)
    	{
    		accountflag=0;
    	}
    }
    ui->username->setPlaceholderText(tr("Username"));
    if(m_remember_me)//remember me checked
    {
    	//target count not zero
    	if(ui->targetcomboBox->count()!=0)
    	{
           // qUsername=QString(host_info[0].username);
		    //qUsername.replace("\\","\\\\");			
            //ui->username->setText(qUsername);
    	}
    }    
    ui->password->setPlaceholderText(tr("Password"));
    ui->password->clear();//clear password field
    //===============================================
    m_kb_fd = open("/dev/tty0", O_NOCTTY|O_ASYNC);
    if(m_kb_fd)
    {    	
        ioctl(m_kb_fd, KDGETLED, &m_KBD_status);
        ui->label_8->clear();
        m_KBD_status |= NumLock_ON;
        m_KBD_status &= ~CapsLock_ON;
        ioctl(m_kb_fd, KDSETLED, m_KBD_status);       
    }   
    rdpf = fopen(RDP_CONF_PATH, "rt");
    if(rdpf)
    {
        fgets(m_cmdline, CMDMAX, rdpf);
        fclose(rdpf);
        rdpf=0;
    }   
    checkCRTstatus();
    customer_init();
    init_dialog();   
    ui->targetcomboBox->installEventFilter(this);
    ui->commandLinkButton->installEventFilter(this);
    ui->username->installEventFilter(this);
    ui->password->installEventFilter(this);
    
    ui->targetcomboBox->view()->viewport()->installEventFilter(this);
    connect(ui->targetcomboBox, SIGNAL(highlighted(int)), this, SLOT(onTargetHighlighted(int)));
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timer_timeout()));
    m_timer->start(3600);    
    ui->configure->setText(tr("Configure"));
    ui->commandLinkButton->setText(tr("Login"));
    //crt_timer->singleShot(0, tno monitor connectionhis,SLOT(crt_timeout()));
    #ifdef across
        QWSServer::setCursorVisible(true);
    #endif
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent, Qt::FramelessWindowHint),    
    ui(new Ui::MainWindow)
{
	char prestr[10] = {0};	 
	ui->setupUi(this);
	setObjectName("connectui");
	m_rlefd = open("/dev/access", O_RDWR|O_ASYNC);   
    m_gpiofd = open("/dev/ast-gpio", O_RDWR|O_ASYNC);   
    getBoardInfo(&g_board_info);
    printf("ui:Board Ver:%x\n", g_board_info);
	if((g_board_info & (1<<SCRACH_CRT_A_ON_BIT)) && (g_board_info & (1<<SCRACH_CRT_B_ON_BIT)))
	{
		m_multiMonitor = 1;

		if(pUtility->readfromfile("CRT_ON_IDX", prestr, UICONF_CONF_PATH))
		{
			m_crtidx = atoi(prestr);
		}
		else
			m_crtidx = CRT_A;
	}
	else if(g_board_info & (1<<SCRACH_CRT_A_ON_BIT))
	{
		m_crtidx = CRT_A;
		m_multiMonitor = 0;
	}
	else if(g_board_info & (1<<SCRACH_CRT_B_ON_BIT))
	{
		m_crtidx = CRT_B;
		m_multiMonitor = 0;
	}else{
		printf("ui:no monitor connection!\n");
    }
    TurnOnCRT(0);
    TurnOnCRT(1);

    crt_timer = new QTimer(this);
    crt_timer->singleShot(0, this,SLOT(initial_main()));
}

int MainWindow::createlabel(p_customer pdata)
{
	char dataline[MAX_COMMAND_LENGTH];
	QLabel *newlabel=new QLabel(this);
    QFont newfont;
    newfont=newlabel->font();
    if(pdata->flag==LABELSTART)
    {
            newlabel->setText(tr(pdata->cname));
    }
    if(pdata->flag==DATASTART)
    {
    		memset(dataline,0,MAX_COMMAND_LENGTH);
    		if(pUtility->readfromfile(pdata->cname,dataline,pdata->cpath))
    		{
    				newlabel->setText(tr(dataline));
    		}
    }    
    newlabel->setGeometry(pdata->cgeo[0],pdata->cgeo[1],pdata->cgeo[2],pdata->cgeo[3]);
	newfont.setPointSize(pdata->cfontsize);
	newlabel->setFont(newfont);
	newlabel->show();
	return 0;
}
int MainWindow::customer_init()
{
	char dataline[MAX_COMMAND_LENGTH];
	int linecount=0;
	int i=0;
	int label_flag=0;
	char *token;
	char *token1;
	const char stok[2] = "=";
	const char stok1[2] = ",";
	m_customer mtagdata;
	//check file exist 
	if(pUtility->file_exist(CUSTOMER_PATH))
	{
			//get how many lines in file
			linecount=pUtility->get_line_count(CUSTOMER_PATH);
			//read line by line
			for(i=0;i<linecount;i++)
			{
					memset(dataline,0,MAX_COMMAND_LENGTH);
					if(pUtility->read_line_from_file(i+1,dataline,CUSTOMER_PATH))
					{
						if(strlen(dataline)>0)
						{ 
							dataline[strlen(dataline)]=0;
							if(label_flag==TAGEND)
							{
									if(!strcmp(dataline,"[LABEL_START]"))
									{
											label_flag=LABELSTART;
											memset((char*)&mtagdata,0,sizeof(m_customer));
											mtagdata.flag=LABELSTART;
									}
									if(!strcmp(dataline,"[DATA_START]"))
									{
											label_flag=DATASTART;
											memset((char*)&mtagdata,0,sizeof(m_customer));
											mtagdata.flag=DATASTART;
									}
							}
							else
							{
									//check if tag end
									if(!strcmp(dataline,"[TAG_END]"))
									{
											label_flag=TAGEND;
											//create label
											createlabel(&mtagdata);
									}
									else
									{ //handle data line
									   //get the first data
									    token = strtok(dataline, stok);
										if(!strcmp(token,"NAME"))
										{
											token = strtok(NULL, stok);	
											//mtagdata.cname
											memcpy(mtagdata.cname,token,strlen(token));
										}
										if(!strcmp(token,"FONTSIZE"))
										{
											token = strtok(NULL, stok);	
											mtagdata.cfontsize=atoi(token);
										}	
										if(!strcmp(token,"GEOMETRY"))
										{
											token = strtok(NULL, stok);	
											token1 = strtok(token, stok1);
											//get x
											mtagdata.cgeo[0]=atoi(token1);
											token1 = strtok(NULL, stok1);
											//get y
											mtagdata.cgeo[1]=atoi(token1);
											token1 = strtok(NULL, stok1);
											//get w
											mtagdata.cgeo[2]=atoi(token1);
											token1 = strtok(NULL, stok1);
											//get h
											mtagdata.cgeo[3]=atoi(token1);										
                                        }
										if(label_flag==DATASTART)
										{
												if(!strcmp(token,"PATH"))
												{
													token = strtok(NULL, stok);	

													//mtagdata.cname
													memcpy(mtagdata.cpath,token,strlen(token));
												}		
										}
									}
							}
						}
					}
			}
	}
		return 0;
}
void MainWindow::init_dialog()
{
//    //general settings
//    settingwindow = new SettingDialog(this);
//    connect(settingwindow, SIGNAL(accepted()),this, SLOT(onDLGGENERALFLAG()));
//    connect(settingwindow, SIGNAL(autodetectRES(int)),this, SLOT(autoDetectMode(int)));
//    connect(this, SIGNAL(ResDetect(int)),settingwindow, SLOT(onResDetect(int)));
//    //connect(settingwindow, SIGNAL(Rememberme(int)),this, SLOT(onRememberme(int)));
//    //login dialog

//    //advance dialog
//    advancewindow = new AdvanceDialog(this);
//    connect(advancewindow, SIGNAL(change_dhcpmode(int)),this, SLOT(onchange_dhcpmode(int)));
//    connect(this, SIGNAL(autodhcp()),advancewindow, SLOT(onautodhcp()));
//    connect(this, SIGNAL(NetInvalid()),advancewindow, SLOT(onNetInvalid()));
//    connect(advancewindow, SIGNAL(accepted()),this, SLOT(onDLGADVANCEFLAG()));
//    //autologin and remember me move to general settings

//    //tools dialog
//    toolwindow = new ToolDialog(this);
//    connect(toolwindow, SIGNAL(accepted()),this, SLOT(onDLGTOOLFLAG()));

//    //system information
//    infowindow = new InfoDialog(this);
//    //connect(this, SIGNAL(autodhcp()),infowindow, SLOT(onautodhcp()));
//    //connect(this, SIGNAL(NetInvalid()),infowindow, SLOT(onNetInvalid()));
//    connect(infowindow, SIGNAL(accepted()),this, SLOT(onDLGINFOFLAG()));

//    //autologin and remember connect to load default
//    connect(advancewindow, SIGNAL(loaddefault()),settingwindow, SLOT(onAutoLoginstate()));
    Init_IP_Dialog();
   	mainDialog = new MainDialog(this);
    connect(mainDialog,SIGNAL(back_pressed()),this,SLOT(Init_IP_Dialog())); 
    connect(mainDialog,SIGNAL(changeLanguage(int)),this,SLOT(changeLanguage(int)));
    connect(mainDialog,SIGNAL(WritetoRdpconftoMain(QString,QString,QString)),this,SLOT(on_WritetoRdpconf_Main(QString,QString,QString)));
    connect(mainDialog,SIGNAL(m_autodetectRES(int)),this, SLOT(autoDetectMode(int)));


}
int MainWindow::show_last_connect()
{
	int ret=0;	
    char cmdline[MAX_COMMAND_LENGTH];
    manual_flag = false;
    if(pUtility->file_exist(TMP_HOST_DATA))
    {
    	memset(cmdline,0,MAX_COMMAND_LENGTH);	
    	//read last host name or ipaddress and show
		if(pUtility->readfromfile("host",cmdline,TMP_HOST_DATA))
		{
            qDebug()<<"show_last_connect: "<<QString(cmdline);
            if(QString(cmdline)==QString("Manual"))
            {
                manual_flag = true;
                if(m_remember_me)
                {
                    //read last username and show
                    memset(cmdline,0,MAX_COMMAND_LENGTH);
                    if(pUtility->readfromfile("username",cmdline,TMP_HOST_DATA))
                    {
                       on_targetcomboBox_activated(0);
                       ui->targetcomboBox->setEditText(userinfo.value("ConnectName0").toString());
                       ui->username->setText(userinfo.value("Username0").toString());
                       ui->password->setText(userinfo.value("Password0").toString());

                    }
                }
                else
                {
                    ui->targetcomboBox->setCurrentIndex(1);
                    ui->targetcomboBox->setEditText(userinfo.value("ConnectName1").toString());
                    ui->username->setText(userinfo.value("Username1").toString());
                    ui->password->setText(userinfo.value("Password1").toString());
                }

            }
            else
            {
                manual_flag = false;
                on_targetcomboBox_currentIndexChanged(QString(cmdline));
            }
		}


    }
    else
    {
        ui->targetcomboBox->setCurrentIndex(1);
        ui->targetcomboBox->setEditText(userinfo.value("ConnectName1").toString());
        ui->username->setText(userinfo.value("Username1").toString());
        ui->password->setText(userinfo.value("Password1").toString());
    }

	return ret;
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::saveresolution(int size)
{
   char olddata[4];
   char newdata[4];
   memset(olddata,0,4);
   memset(newdata,0,4);
    if(pUtility->readfromfile("SIZE",olddata,SETTING_PATH))
    {
        sprintf(newdata,"%d",size);
		if(pUtility->savetofile("SIZE",olddata,newdata,SETTING_PATH))
		{
		}        
    }   
}

int MainWindow::autoDetectMode(int b_multi)
{
	int modeidx;
    int found;
    char mystr[4] = {0};
    int multi_monitor=0;
    qDebug()<<"MainWindow: autoDetectMode";
    memset(mystr,0,4);
    if(pUtility->readfromfile(MULTI_MONITOR,mystr,SETTING_PATH))
    {
        multi_monitor=atoi(mystr);
    }

    if(multi_monitor==1)
        return 0;

    found = get_prefer_mode_idx(m_crtidx, modeidx);//single monitor

    if(MODE_NOCON == found)
    {
        if(b_multi)
        {
            get_prefer_mode_idx(!m_crtidx, modeidx);
        }
    }

    return modeidx;
}

int MainWindow::get_prefer_mode(PREFER_MODE* pmode)
{
	return ioctl(m_rlefd, IOCTL_GET_PREFER_MODE, pmode);
}

int MainWindow::mode_check(PREFER_MODE* p_mode)
{
	int bChangeType = MODE_FOUND;
	
	//printf("mode_check:w:%d, h:%d\n", p_mode->width, p_mode->height);
	
	if(p_mode->width >= 1920 && p_mode->height >= 1200 && p_mode->width <= 3840 && p_mode->height <= 2400)
	{
		p_mode->width = 1920;	//set to 1920x1080 mode
		p_mode->height = 1080;	
		bChangeType = MODE_OVER;
	}
	else if(p_mode->width == 1366 || p_mode->height == 768)
	{
		p_mode->width = 1360;	
		p_mode->height = 768;	
		bChangeType = MODE_CHANGE;
	}
	else if(p_mode->width <= 0 || p_mode->height <= 0)
	{	
		p_mode->width = 1024;	
		p_mode->height = 768;	
		bChangeType = MODE_NOCON;			//no connected
	}

	return bChangeType;
}

int MainWindow::get_prefer_mode_idx(int crtIdx, int &modeidx)
{
    int ret = MODE_FOUND;
    int modetype;
    int j = 0;
    char tmpstr[32] = {0};
    PREFER_MODE prefer_mode;
	prefer_mode.port = crtIdx;
	get_prefer_mode(&prefer_mode);
	modetype = mode_check(&prefer_mode);
	memset(tmpstr,0,32);
	sprintf(tmpstr, "%dx%d", prefer_mode.width, prefer_mode.height);
	for(j=0;j<k_mode_num;j++)
	{
		if(!strcmp(g_mode[j].toLatin1().data(),tmpstr))
		{
			modeidx = j;
			break;
		}
	}
    
    if((dislog_ctrl&DLGCTRL_GENERAL_SETTINGS) == DLGCTRL_GENERAL_SETTINGS)
    {
    	emit ResDetect(j);
    }    

	if(j == k_mode_num)
	{
		ret = MODE_NEWM;	
	}
		
	if(modetype == MODE_NOCON)
	{
		ret = MODE_NOCON;
	}
   
    return ret;
}
void MainWindow::on_commandLinkButton_clicked(bool checked)
{
    FILE*   rdpf = 0;
    //FILE*   crtf = 0;
    char    mystr[4] = {0};    
    int     performance;
    char    cmmandlink[MAX_COMMANDLINK_LENGTH];
    char*   pcmd;
    int     resolution;
    int		multi_monitor;
    int     waittime;
    int     breakhotkey;
    int	    readvalue;
    QString qTmpstr;
    QString rememberstr;
    int	    cmdlen=0;
    int	    autosize=0;
    int		bandwidth=0;
    int     audio=0;
    char    cmdline[MAX_COMMAND_LENGTH];
    FILE    *hopen;
    char    ret[255];
    //add for station name
    char    devname[128];
    //add for dual monitor
    PREFER_MODE multi_resolution[2];

    qDebug()<<"on_commandLinkButton_clicked Login...";

    memset(cmmandlink,0,MAX_COMMANDLINK_LENGTH);
    pcmd = cmmandlink;    
#if 0 
    performance= ui->Slider_QPset->value();
#else
    performance=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("USERPERF",mystr,SETTING_PATH))
    {
        performance=atoi(mystr);
    }    
#endif
    if("" == ui->targetcomboBox->currentText())
    {
        QMessageBox advmsg;
        advmsg.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
        advmsg.setText(tr("Pleasse input IP address rightful!"));
        advmsg.exec();
        return;
    }
    if(manual_flag)
    if("Manual" != ui->targetcomboBox->currentText())
    {
        QRegExp rx("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(\\:(([1-6][0-9][0-9][0-9][0-9])|([1-9][0-9][0-9][0-9])|([1-9][0-9][0-9])|([1-9]?[0-9])))?");

        if( !rx.exactMatch( ui->targetcomboBox->currentText() ) )
        {
            QMessageBox advmsg;
            advmsg.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
            advmsg.setText(tr("Pleasse input IP address rightful!"));
            advmsg.exec();
            ui->targetcomboBox->setEditText("");
            return;
        }
    }

    this->setEnabled(false);

    if(pUtility->file_exist(TMP_HOST_DATA))
    {
        system("rm -rf /usr/ast/tmphost");
        system("sync");

    }

    qTmpstr=ui->username->text();
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    //add for last connect username
    rememberstr=qTmpstr;
    rememberstr.replace("\\\\","\\\\\\\\");
	if(pUtility->savetofile("username",NULL,rememberstr.toLatin1().data(),TMP_HOST_DATA))
    //if(pUtility->savetofile("USER",NULL,qTmpstr.toLatin1().data(),TMP_HOST_DATA))
	{
		//qDebug("(%s:%d)user %s\n",__FILE__,__LINE__,rememberstr);
	}        
    sprintf(pcmd+cmdlen,"/usr/local/bin/sfreerdp /u:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);
    
    qTmpstr=ui->password->text();
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    sprintf(pcmd+cmdlen,"/p:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);    
    
    memset(mystr,0,4);
    if(pUtility->readfromfile("BANDWIDTH", mystr, SETTING_PATH))
    {
    	bandwidth = atoi(mystr);
    }
    switch(bandwidth)
    {
	case 0:
	{
		//qDebug("rdp8\n");
		sprintf(pcmd+cmdlen,"/rdp8 /dvc:ast3200 /network:auto ");
		cmdlen=strlen(pcmd);       	
	}
	break;
	case 1:
	case 2:
	default:
	{
		//qDebug("rdp7.1\n");
		sprintf(pcmd+cmdlen,"/rfx /network:lan ");
		cmdlen=strlen(pcmd);    	
	}
	break;
	}
        
    //Audio command
    memset(mystr,0,4);
    if(pUtility->readfromfile("AUDIO",mystr,SETTING_PATH)){
        audio=atoi(mystr);
    }
    if(audio){
        sprintf(pcmd+cmdlen,"/sec:rdp /vc:rdpsnd,sys:alsa,dev:default,rate:44100,channel:2,latency:200 /microphone:sys:alsa,dev:default ");
        cmdlen=strlen(pcmd);
    }

    switch(performance)
    {
        case 0:
            qTmpstr="1";
        break;
        case 1:
            qTmpstr="1";
        break;
        case 2:
            qTmpstr="2";
        break;
        case 3:
            qTmpstr="3";
        break;
        case 4:
            qTmpstr="4";
        break;
    }    
    sprintf(pcmd+cmdlen,"/frame-ack:%s ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);       
    //check dual monitor parameter
    multi_monitor=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile(MULTI_MONITOR,mystr,SETTING_PATH))
    {
    	multi_monitor=atoi(mystr);
    }
    //printf("multi_monitor=%d\n",multi_monitor);
    if(multi_monitor==0)
    { //single monitor
	#if 0
		//q_cmdline.append(ui->comboBox_1->currentText());
	#else
		memset(mystr,0,4);
		resolution=0;
		if(pUtility->readfromfile("AUTORESOLUTION",mystr,SETTING_PATH))
		{
			autosize=atoi(mystr);
			if(autosize)
			{
				resolution = autoDetectMode(m_crtidx);
			}
			else
			{
				memset(mystr,0,4);
				if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
				{
					resolution = atoi(mystr);
				}          		
			}
		}         
	#endif
		qTmpstr=g_mode[resolution];
		sprintf(pcmd+cmdlen,"/size:%s ",qTmpstr.toLatin1().data());
		cmdlen=strlen(pcmd);    
		
        memset(cmdline,0,MAX_COMMAND_LENGTH); 
        if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
        {
            sprintf(pcmd+cmdlen, "/mirror ");
            cmdlen=strlen(pcmd);         
            //printf("connect: %s\n", pcmd);
        } 			
    }
    else
    { //dual monitor
    	multi_resolution[0].port = CRT_A;
    	multi_resolution[1].port = CRT_B;
    	/*if(m_crtidx == CRT_A)
    	{
    		multi_resolution[1].port = CRT_B;
    	}
    	else if(m_crtidx == CRT_B)
    	{
    		multi_resolution[1].port = CRT_A;
        } */


        if(pUtility->readfromfile("AUTORESOLUTION",mystr,SETTING_PATH))
        {
            autosize=atoi(mystr);
            if(autosize)
            {
                //resolution = autoDetectMode(m_crtidx);
                get_prefer_mode(&multi_resolution[0]);
                mode_check(&multi_resolution[0]);
                get_prefer_mode(&multi_resolution[1]);
                mode_check(&multi_resolution[1]);
            }
            else
            {
                memset(mystr,0,4);
                if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
                {
                    resolution = atoi(mystr);
                    multi_resolution[0].width=g_mode_w[resolution];
                    multi_resolution[0].height=g_mode_h[resolution];
                }
                else
                {
                    multi_resolution[0].width=1024;
                    multi_resolution[0].height=768;
                }

            }
        }
        else
        {
            memset(mystr,0,4);
            if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
            {
                resolution = atoi(mystr);
                multi_resolution[0].width=g_mode_w[resolution];
                multi_resolution[0].height=g_mode_h[resolution];
            }
            else
            {
                multi_resolution[0].width=1024;
                multi_resolution[0].height=768;
            }
        }

        //get_prefer_mode(&multi_resolution[1]);
        //mode_check(&multi_resolution[1]);
    	
        sprintf(pcmd+cmdlen,"/monitordef:0,0,%d,%d-1:%d,0,%d,%d-0 ",multi_resolution[0].width, multi_resolution[0].height, multi_resolution[0].width, multi_resolution[0].width, multi_resolution[0].height);
    	cmdlen = strlen(pcmd);
    }

    memset(mystr,0,4);
    if(pUtility->readfromfile("EVOR",mystr,SETTING_PATH)){
        if(0==atoi(mystr)){
            sprintf(pcmd+cmdlen,"/evor ");
            cmdlen = strlen(pcmd);
        }
    }

	sprintf(pcmd+cmdlen,"/sec:rdp ");

    cmdlen=strlen(pcmd);    
    memset(mystr,0,4);
    
    sprintf(pcmd+cmdlen,"/userperf:%d ",performance);
    cmdlen=strlen(pcmd);     
#if 0
    sprintf(mystr, "%d", ui->spinBox_1->value());
#else
    waittime=33;
    memset(mystr,0,4);
    if(pUtility->readfromfile("WAITSECS",mystr,SETTING_PATH))
    {
        waittime=atoi(mystr);
    }    
    sprintf(mystr, "%d", waittime);
#endif
    sprintf(pcmd+cmdlen,"/waitsecs:%d ",waittime);
    cmdlen=strlen(pcmd);    
    breakhotkey=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("BKHOTKEY",mystr,SETTING_PATH))
    {
        breakhotkey=atoi(mystr);
    }  
    sprintf(pcmd+cmdlen,"/bkhotkey:%d ",breakhotkey);
    
    cmdlen=strlen(pcmd);     
    

    if(manual_flag)
    {

        userinfo.setValue(QString("ConnectName0"),QString("Manual"));
        if(ui->targetcomboBox->currentText()!=QString("Manual"))
        {
            userinfo.setValue(QString("IPAddress0"),ui->targetcomboBox->currentText());           
            qTmpstr=ui->targetcomboBox->currentText();

            qTmpstr.replace("\"","\\\"");

        }
        else
        {
            qTmpstr=userinfo.value("IPAddress0").toString();
            qTmpstr.replace("\"","\\\"");
        }

        userinfo.setValue(QString("Username0"),ui->username->text());
        userinfo.setValue(QString("Password0"),ui->password->text());
        if(pUtility->savetofile("host",NULL,"Manual",TMP_HOST_DATA))
        {

        }
    }
    else
    {
        //qTmpstr=ui->targetcomboBox->currentText();
        qTmpstr=current_hostip;
        //qTmpstr.replace("\\","\\\\");
        qTmpstr.replace("\"","\\\"");
        //add for save host to /tmp/tmphost
        rememberstr=ui->targetcomboBox->currentText().replace("\"","\\\"");
        //rememberstr.replace("\\","\\\\");
        if(pUtility->savetofile("host",NULL,rememberstr.toLatin1().data(),TMP_HOST_DATA))
        {

        }

    }
    sprintf(pcmd+cmdlen,"/v:\"%s\"",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd); 

    //usbfunction enable or disable
    readvalue=1;
    memset(mystr,0,4);
    if(pUtility->readfromfile("USBSTORAGE",mystr,USB_CONFIG_PATH))
    {
        readvalue=atoi(mystr);       
    } 
    if(readvalue==1)
    {
    	//if usbfuction enable
    	sprintf(pcmd+cmdlen," /drive:*,hotplug");
        cmdlen=strlen(pcmd);
    }
    //*****Check urbdrc is enabled or disabled*****
    readvalue=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("URBDRC",mystr,URBDRC_CONFIG_PATH)){
        readvalue=atoi(mystr);
    }

    if(readvalue==1){

        memset(cmdline,0,MAX_COMMAND_LENGTH);
        memset(ret,0,255);
        sprintf(cmdline,"cat /usr/ast/urbdrc_devices.conf");
        hopen=popen(cmdline,"r");
        while(fgets(ret, sizeof(ret) - 1, hopen) != NULL){}

        sprintf(pcmd+cmdlen," %s", ret);
        cmdlen=strlen(pcmd);
    }

    //*****Check smartcard is enabled or disabled*****

    readvalue=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("SMARTCARD",mystr,SMARTCARD_CONFIG_PATH)){
        readvalue=atoi(mystr);
    }

    if(readvalue==1){
        sprintf(pcmd+cmdlen," /smartcard:");
        cmdlen=strlen(pcmd);
    }
    
    //add for station name===========================
    memset(devname,0,128);
    if(!pUtility->readfromenv("devname", devname))
    {
        if(!pUtility->readfromfile("DEVICE_NAME", devname, SYSTEM_CONFIG_PATH))
        {
        	//use PC Extender
        	sprintf(devname,"PC Extender");
        }
    }
    
    qTmpstr=QString(devname);
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    sprintf(pcmd+cmdlen," /devname:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);     
    //=============================================================

    showMaximized();
    setEnabled(true);
    rdpf = fopen(RDP_CONF_PATH, "w");
    if(rdpf)
    {
        fputs(pcmd, rdpf);
        fclose(rdpf);
        rdpf=0;
        system("sync");
    }    
    if(m_kb_fd)
    {
        ::close(m_kb_fd);
        m_kb_fd = 0;
    }
    io_CRT.Address = 0x1e6e6000 + (m_crtidx<<8) + 0x80;
    io_CRT.Data = VIDEOMEM_BASE;
    ioctl (m_rlefd, IOCTL_IO_WRITE, &io_CRT);
    if(false == checked)
        exit(1);
}

void MainWindow::onchange_dhcpmode(int mode)
{	
    char cmdline[MAX_COMMAND_LENGTH];	
    memset(cmdline,0,MAX_COMMAND_LENGTH);
    m_bUseDHCP=mode;	   
}
void MainWindow::onDHCPdone()
{	
    char cmdline[MAX_COMMAND_LENGTH];	
    memset(cmdline,0,MAX_COMMAND_LENGTH);   	
    emit autodhcp();
}
int MainWindow::getIPaddr(char* addr)
{
    FILE *fconf;
    char cmdline[MAX_COMMAND_LENGTH];
    int ret=0;
    //read mac address
    memset(cmdline,0,MAX_COMMAND_LENGTH);	
    sprintf(cmdline,"ifconfig | awk '/inet addr/{print substr($2,6)}'");
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
	memset(cmdline,0,MAX_COMMAND_LENGTH);
	if(fgets(cmdline,20, fconf) != NULL)
	{			            		
	    if(strlen(cmdline)>0)
	    {
		cmdline[strlen(cmdline)-1]=0;		    	    
		ret=1;
		memcpy(addr,cmdline,strlen(cmdline));
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
    return ret;
}

void MainWindow::onLoginfunc()
{	
    mainDialog->setWindowTitle(tr("Administrative Setttings"));
    mainDialog->setParent(this);
    mainDialog->setStyleSheet("background-color:Pale gray;");
    mainDialog->setWindowFlags(Qt::FramelessWindowHint);
    mainDialog->setGeometry(187,184,700,432);
    mainDialog->show();
    mainDialog->mainUiInit();
    mainDialog->on_system_pressed();
}
void MainWindow::onDLGCTRL(int ctrl)
{
	dislog_ctrl&=(~ctrl);	
}

int MainWindow::parse_vhost()
{
    int listcount=0;
    int ret=0;
    int linenum=0;
    char cmdline[MAX_COMMAND_LENGTH];
    char hostdata[MAX_COMMAND_LENGTH];
    char hostname[MAX_COMMAND_LENGTH];
    char *pdata=hostdata;
    QString qstrname;
    memset(cmdline,0,MAX_COMMAND_LENGTH);
    memset(hostdata,0,MAX_COMMAND_LENGTH);
    memset(hostname,0,MAX_COMMAND_LENGTH);
    do
    {
	if(!pUtility->file_exist(VHOST_PATH))
	{
		break;
	}    	    
	//if(pUtility->readfromfile("host",cmdline,VHOST_PATH))
	if(pUtility->readfromfile("host",cmdline,TMP_HOST_DATA))
	{	
		qstrname=QString(cmdline);
		qstrname.replace("\"","\\\"");				
		sprintf(pdata,"host=%s\n",qstrname.toLatin1().data());		
	} 
	else
	{
		break;
	}
	
	memset(cmdline,0,MAX_COMMAND_LENGTH);
	//if(pUtility->readfromfile("username",cmdline,VHOST_PATH))
	if(pUtility->readfromfile("username",cmdline,TMP_HOST_DATA))
	{
		qstrname=QString(cmdline);
		//qstrname.replace("\\","\\\\");
		qstrname.replace("\"","\\\"");	
		qstrname.replace("\\\\","\\\\\\\\");
		if(m_remember_me)
		{
			sprintf(hostname,"username=%s\n",qstrname.toLatin1().data());
		}
		else
		{
			sprintf(hostname,"username=\n");
		}
		//qDebug("%s\n",hostname);
	} 
	else
	{
		break;
	}
	//add host to hostlist
	{
	  //remove vhost
	  remove(VHOST_PATH);	  
	  system("sync");
	  //find the data in hostlist, remove data in old position
	  linenum=pUtility->find_string_infile(hostdata,HOST_LIST_PATH);
	  if(linenum!=0)
	  {
	  	  pUtility->delete_number_line(linenum+1,HOST_LIST_PATH);
	  	  pUtility->delete_number_line(linenum,HOST_LIST_PATH);
	  }	  
	  //check how many item in list.
	  listcount=pUtility->get_line_count(HOST_LIST_PATH);
	  if(listcount == (MAX_HOST_LIST_NUM*2))
	  {
	  	  //if list full, delete teail
	  	  pUtility->delete_number_line(listcount,HOST_LIST_PATH);
	  	  listcount=pUtility->get_line_count(HOST_LIST_PATH);
	  	  pUtility->delete_number_line(listcount,HOST_LIST_PATH);	  	  
	  }
	  //insert data to first line
	  if(pUtility->insert_head(hostname,HOST_LIST_PATH))
	  {
	  	  ret=1;
	  }	
	  if(pUtility->insert_head(hostdata,HOST_LIST_PATH))
	  {
	  	  ret=1;
	  }		
	}
    }while(0);
    return ret;
}


int MainWindow::is_ip_exist(char* ipstr)
{
	int ret=0;
	FILE *fconf;
	char cmdline[MAX_COMMAND_LENGTH];	
	do
	{
		memset(cmdline,0,MAX_COMMAND_LENGTH);
		sprintf(cmdline,"grep -c \"%s\" %s",ipstr,HOST_LIST_PATH);;
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			memset(cmdline,0,MAX_COMMAND_LENGTH);    
			if(fgets(cmdline,MAX_COMMAND_LENGTH-1, fconf) != NULL)
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
	}while(0);
	return ret;
}
void MainWindow::add_managerip()
{
	int linecount=0;
	int lineindex=1;
	char* ptrchr=NULL;
	char* ptrpos=NULL;
	char cmdline[MAX_COMMAND_LENGTH];
    memset(cmdline,0,MAX_COMMAND_LENGTH);	
	linecount=pUtility->get_line_count("/tmp/maniplist");
	if(linecount>0)
	{
	    while(lineindex <= linecount)
		{
			if(pUtility->read_line_from_file(lineindex,cmdline,"/tmp/maniplist"))
			{
				ptrpos=cmdline;
				ptrchr=strstr(ptrpos,"http://");
				if(ptrchr!=NULL)
				{
					ptrpos+=7;
					ptrchr=strstr(ptrpos,"/");
					if(ptrchr!=NULL)
					{					
						*ptrchr=0;
					}
					//check this ip not in hostlist
					if(pUtility->file_exist(HOST_LIST_PATH))
					{
						if(!(is_ip_exist(ptrpos)))
						{
							ui->targetcomboBox->addItem(QString(ptrpos));
						}
					}
					else
					{
						ui->targetcomboBox->addItem(QString(ptrpos));
					}					
				}
				else
				{
					//qDebug("Not found\n");
				}
			}
			lineindex++;
			if(lineindex==3)//only read two manager ip address
			{
				break;
			}
		}
	}	
}
int MainWindow::read_rememeber_me()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LENGTH];
    if(pUtility->readfromfile("REMEMBER_ME",cmdline,SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;	
}

void MainWindow::on_targetcomboBox_currentIndexChanged(int index)
{
//    QString qUsername;
    qDebug()<<"on_targetcomboBox_currentIndexChanged int..."<<index;
//    if(combo_flag)
//    {
//        if((index==0) && m_remember_me)
//        {
//            qUsername=QString(host_info[index].username);
//            //qUsername.replace("\\","\\\\");
//           // ui->username->setText(qUsername);
//        }
//        else
//        {
//            ui->username->clear();
//        }
//        ui->password->clear();
//    }
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    char cmdline[MAX_COMMAND_LENGTH];
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
//  if(obj==advancewindow && advancewindow->isActiveWindow() )
//  {
//  		  advance_idle_counter=0;
//  }
     if (event->type() == QEvent::KeyRelease) 
     {
			if(Qt::Key_CapsLock == keyEvent->key())   //capslock
			{
				if(m_kb_fd)
				{
					ioctl(m_kb_fd, KDGETLED, &m_KBD_status);
					if(m_KBD_status & CapsLock_ON)
					{				
                         ui->label_8->setText(tr("Caps Lock ON"));
					}
					else
					{
                    ui->label_8->clear();
					}
				}
			}     	 
     }
     if (event->type() == QEvent::KeyPress) 
     {
			if((Qt::Key_Return == keyEvent->key()) || (Qt::Key_Enter == keyEvent->key()))  //enter
			{
				if( obj == ui->commandLinkButton ||
					obj == ui->targetcomboBox ||
                    obj == ui->username ||
                    obj == ui->password
					)
				{
					on_commandLinkButton_clicked(false);
				}			
			}                  
     }
     // pass the event on to the parent class
     return QMainWindow::eventFilter(obj, event);	
}
void MainWindow::onDLGWIFIFLAG()
{
	dislog_ctrl&=(~DLGCTRL_WIFI_SETTINGS);	
}
void MainWindow::onDLGINFOFLAG()
{
	dislog_ctrl&=(~DLGCTRL_SYSTEM_INFO);	
	//delete infowindow;
}
void MainWindow::onDLGTOOLFLAG()
{
    qDebug()<<"accepted";
	dislog_ctrl&=(~DLGCTRL_TOOLS);
	//delete toolwindow;
}
void MainWindow::onDLGLOGFLAG()
{
	//dislog_ctrl&=(~DLGCTRL_SYSTEM_LOG);
}
void MainWindow::onDLGADVANCEFLAG()
{
	dislog_ctrl&=(~DLGCTRL_ADVANCE_SETTINGS);
	//delete advancewindow;
}
void MainWindow::onDLGGENERALFLAG()
{

	dislog_ctrl&=(~DLGCTRL_GENERAL_SETTINGS);
    //delete settingwindow;
}
void MainWindow::onDLGLOGINFLAG()
{
	dislog_ctrl&=(~DLGCTRL_ADVANCE_LOGIN);
	//delete loginwindow;
}
void MainWindow::onTargetHighlighted(int index)
{
	hostindex=index;
}
void MainWindow::crt_timeout()//20151119
{
    //printf("uitest:%x\n", g_board_info);
    if(g_board_info & (1<<SCRACH_CRT_A_ON_BIT))
        TurnOnCRT(CRT_A);
	if(g_board_info & (1<<SCRACH_CRT_B_ON_BIT))
        TurnOnCRT(CRT_B);
}


void MainWindow::on_configure_clicked()
{
    //mainDialog->setGeometry(220,200,700,500);
    char tmpbuf[CMDMAX];
    char uname[MAX_STR_LEN];
    char pword[MAX_STR_LEN];
    if(!(dislog_ctrl&DLGCTRL_ADVANCE_SETTINGS))
    {
            //check username/password both empty, then call onLoginfunc()
        memset(uname,0,MAX_STR_LEN);
        memset(pword,0,MAX_STR_LEN);
        //get username
        pUtility->readfromfile("USERNAME",uname,ACCOUNT_CONFIG_PATH);
        //get password
        pUtility->readfromfile("PASSWORD",pword,ACCOUNT_CONFIG_PATH);
        if((strlen(uname)==0)&&(strlen(pword)==0))
        {
            qDebug()<<"on_configure_clicked-->onLoginfunc Configure...";
            onLoginfunc();
        }
        else
        {

        }
    }
    else
    {
       // mainDialog->setGeometry(220,200,700,500);
        mainDialog->raise();
        mainDialog->activateWindow();
    }
}
void MainWindow::on_targetcomboBox_currentIndexChanged(const QString &current_ConnectName)
{
    QStringList myOptions;
    int current_Name_state;
    ui->username->clear();
    ui->password->clear();
    qDebug()<<"on_targetcomboBox_currentIndexChanged QString..."<<current_ConnectName;
    myOptions << userinfo.value("ConnectName1").toString()<< userinfo.value("ConnectName2").toString() << userinfo.value("ConnectName3").toString()<<userinfo.value("ConnectName4").toString()<<userinfo.value("ConnectName5").toString()<<userinfo.value("ConnectName6").toString();
    current_Name_state = myOptions.indexOf(current_ConnectName);
    ui->targetcomboBox->setEditable(true);
    ui->username->setEnabled(true);
    ui->password->setEnabled(true);
    ui->targetcomboBox->setCurrentIndex(current_Name_state+1);
    switch(current_Name_state){
        case 0:

            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName1").toString());
            ui->username->setText(userinfo.value("Username1").toString());
            ui->password->setText(userinfo.value("Password1").toString());
            current_hostip=userinfo.value("IPAddress1").toString();
        break;
        case 1:
            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName2").toString());
            ui->username->setText(userinfo.value("Username2").toString());
            ui->password->setText(userinfo.value("Password2").toString());
            current_hostip=userinfo.value("IPAddress2").toString();
        break;
        case 2:
            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName3").toString());
            ui->username->setText(userinfo.value("Username3").toString());
            ui->password->setText(userinfo.value("Password3").toString());
            current_hostip=userinfo.value("IPAddress3").toString();
        break;
        case 3:
            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName4").toString());
            ui->username->setText(userinfo.value("Username4").toString());
            ui->password->setText(userinfo.value("Password4").toString());
            current_hostip=userinfo.value("IPAddress4").toString();
        break;
        case 4:
            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName5").toString());
            ui->username->setText(userinfo.value("Username5").toString());
            ui->password->setText(userinfo.value("Password5").toString());
            current_hostip=userinfo.value("IPAddress5").toString();
        break;
        case 5:
            manual_flag = false;
            ui->targetcomboBox->setEditText(userinfo.value("ConnectName6").toString());
            ui->username->setText(userinfo.value("Username6").toString());
            ui->password->setText(userinfo.value("Password6").toString());
            current_hostip=userinfo.value("IPAddress6").toString();
        break;

        default:
            manual_flag = true;
            break;
    }
    if(ui->password->text()!="")
    {
        ui->targetcomboBox->setEditable(false);
        ui->username->setEnabled(false);
        ui->password->setEnabled(false);
    }
    else
    {
        ui->targetcomboBox->setEditable(false);
        ui->username->setEnabled(true);
        ui->password->setEnabled(true);
    }
    qDebug()<<"current_Name_state: "<<current_Name_state;
    return;
}

void MainWindow::Init_IP_Dialog()
{
    qDebug()<<"Init_IP_Dialog";
    ui->targetcomboBox->clear();
    ui->targetcomboBox->addItem(tr("Input Host Manually"));
    QString IPList;
    for(int i = 1;i < 7;i++){
        IPList = userinfo.value("ConnectName"+QString::number(i,10)).toString();
        if(!IPList.isEmpty()){
            ui->targetcomboBox->addItem(IPList);
        }
    }
    show_last_connect();
}

void MainWindow::changeLanguage(int language)
{
    qDebug()<<"MainWindow changeLanguage: "<<language;
    qDebug()<<"MainWindow current_language: "<<current_language;
    if(current_language == language)
    {
        return;
    }
    current_language = language;

    switch(language)
    {
        case 0:
            translator->load(QString("/usr/ast/cn.qm"));
            break;

        case 1:
            translator->load(QString(":/usr/ast/en.qm"));
            break;

        default:
            translator->load(QString("/usr/ast/cn.qm"));
    }

    mainDialog->translateLanguage();

    ui->retranslateUi(this);
    ui->configure->setText(tr("Configure"));
    ui->commandLinkButton->setText(tr("Login"));
}

void MainWindow::setTranslator(QTranslator *translator, int language)
{
    this->translator = translator;
    current_language = language;
    qDebug()<<"Init current_language: "<<current_language;
}

void MainWindow::on_targetcomboBox_activated(int index)
{
    qDebug()<<"on_targetcomboBox_activated int..."<<index;
    if(0==index) {
        manual_flag = true;
        ui->targetcomboBox->setEditable(true);
        ui->username->setEnabled(true);
        ui->password->setEnabled(true);
        ui->targetcomboBox->setEditText("");
        //ui->targetcomboBox->lineEdit()->setPlaceholderText("input host ip");
        ui->username->clear();
        ui->password->clear();
        return;
    }
    qDebug()<<"on_targetcomboBox_activated int...end";
}

void MainWindow::on_WritetoRdpconf_Main(QString xipaddr, QString xusername, QString xpassword)
{
    FILE*   rdpf = 0;
    //FILE*   crtf = 0;
    char    mystr[4] = {0};
    int     performance;
    char    cmmandlink[MAX_COMMANDLINK_LENGTH];
    char*   pcmd;
    int     resolution;
    int		multi_monitor;
    int     waittime;
    int     breakhotkey;
    int	    readvalue;
    QString qTmpstr;
    QString rememberstr;
    int	    cmdlen=0;
    int	    autosize=0;
    int		bandwidth=0;
    int     audio=0;
    char    cmdline[MAX_COMMAND_LENGTH];
    FILE    *hopen;
    char    ret[255];
    //add for station name
    char    devname[128];
    //add for dual monitor
    PREFER_MODE multi_resolution[2];

    qDebug()<<"on_WritetoRdpconf_Main ..."<<xipaddr<<"   "<<xusername<<"  "<<xpassword;

    memset(cmmandlink,0,MAX_COMMANDLINK_LENGTH);
    pcmd = cmmandlink;
#if 0
    performance= ui->Slider_QPset->value();
#else
    performance=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("USERPERF",mystr,SETTING_PATH))
    {
        performance=atoi(mystr);
    }
#endif


    qTmpstr=xusername;
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    //add for last connect username
    rememberstr=qTmpstr;
    rememberstr.replace("\\\\","\\\\\\\\");

    sprintf(pcmd+cmdlen,"/usr/local/bin/sfreerdp /u:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);

    qTmpstr=xpassword;
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    sprintf(pcmd+cmdlen,"/p:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);

    memset(mystr,0,4);
    if(pUtility->readfromfile("BANDWIDTH", mystr, SETTING_PATH))
    {
        bandwidth = atoi(mystr);
    }
    switch(bandwidth)
    {
    case 0:
    {
        //qDebug("rdp8\n");
        sprintf(pcmd+cmdlen,"/rdp8 /dvc:ast3200 /network:auto ");
        cmdlen=strlen(pcmd);
    }
    break;
    case 1:
    case 2:
    default:
    {
        //qDebug("rdp7.1\n");
        sprintf(pcmd+cmdlen,"/rfx /network:lan ");
        cmdlen=strlen(pcmd);
    }
    break;
    }

    //Audio command
    memset(mystr,0,4);
    if(pUtility->readfromfile("AUDIO",mystr,SETTING_PATH)){
        audio=atoi(mystr);
    }
    if(audio){
        sprintf(pcmd+cmdlen,"/sec:rdp /vc:rdpsnd,sys:alsa,dev:default,rate:44100,channel:2,latency:200 /microphone:sys:alsa,dev:default ");
        cmdlen=strlen(pcmd);
    }

    switch(performance)
    {
        case 0:
            qTmpstr="1";
        break;
        case 1:
            qTmpstr="1";
        break;
        case 2:
            qTmpstr="2";
        break;
        case 3:
            qTmpstr="3";
        break;
        case 4:
            qTmpstr="4";
        break;
    }
    sprintf(pcmd+cmdlen,"/frame-ack:%s ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);
    //check dual monitor parameter
    multi_monitor=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile(MULTI_MONITOR,mystr,SETTING_PATH))
    {
        multi_monitor=atoi(mystr);
    }
    //printf("multi_monitor=%d\n",multi_monitor);
    if(multi_monitor==0)
    { //single monitor
    #if 0
        //q_cmdline.append(ui->comboBox_1->currentText());
    #else
        memset(mystr,0,4);
        resolution=0;
        if(pUtility->readfromfile("AUTORESOLUTION",mystr,SETTING_PATH))
        {
            autosize=atoi(mystr);
            if(autosize)
            {
                resolution = autoDetectMode(m_crtidx);
            }
            else
            {
                memset(mystr,0,4);
                if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
                {
                    resolution = atoi(mystr);
                }
            }
        }
    #endif
        qTmpstr=g_mode[resolution];
        sprintf(pcmd+cmdlen,"/size:%s ",qTmpstr.toLatin1().data());
        cmdlen=strlen(pcmd);

        memset(cmdline,0,MAX_COMMAND_LENGTH);
        if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
        {
            sprintf(pcmd+cmdlen, "/mirror ");
            cmdlen=strlen(pcmd);
            //printf("connect: %s\n", pcmd);
        }
    }
    else
    { //dual monitor
        multi_resolution[0].port = CRT_A;
        multi_resolution[1].port = CRT_B;
        /*if(m_crtidx == CRT_A)
        {
            multi_resolution[1].port = CRT_B;
        }
        else if(m_crtidx == CRT_B)
        {
            multi_resolution[1].port = CRT_A;
        } */

        if(pUtility->readfromfile("AUTORESOLUTION",mystr,SETTING_PATH))
        {
            autosize=atoi(mystr);
            if(autosize)
            {
                //resolution = autoDetectMode(m_crtidx);
                get_prefer_mode(&multi_resolution[0]);
                mode_check(&multi_resolution[0]);
                get_prefer_mode(&multi_resolution[1]);
                mode_check(&multi_resolution[1]);
            }
            else
            {
                memset(mystr,0,4);
                if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
                {
                    resolution = atoi(mystr);
                    multi_resolution[0].width=g_mode_w[resolution];
                    multi_resolution[0].height=g_mode_h[resolution];
                }
                else
                {
                    multi_resolution[0].width=1024;
                    multi_resolution[0].height=768;
                }

            }
        }
        else
        {
            memset(mystr,0,4);
            if(pUtility->readfromfile("SIZE",mystr,SETTING_PATH))
            {
                resolution = atoi(mystr);
                multi_resolution[0].width=g_mode_w[resolution];
                multi_resolution[0].height=g_mode_h[resolution];
            }
            else
            {
                multi_resolution[0].width=1024;
                multi_resolution[0].height=768;
            }
        }

        sprintf(pcmd+cmdlen,"/monitordef:0,0,%d,%d-1:%d,0,%d,%d-0 ",multi_resolution[0].width, multi_resolution[0].height, multi_resolution[0].width, multi_resolution[0].width, multi_resolution[0].height);
        cmdlen = strlen(pcmd);
    }

    memset(mystr,0,4);
    if(pUtility->readfromfile("EVOR",mystr,SETTING_PATH)){
        if(0==atoi(mystr)){
            sprintf(pcmd+cmdlen,"/evor ");
            cmdlen = strlen(pcmd);
        }
    }

    sprintf(pcmd+cmdlen,"/sec:rdp ");

    cmdlen=strlen(pcmd);
    memset(mystr,0,4);

    sprintf(pcmd+cmdlen,"/userperf:%d ",performance);
    cmdlen=strlen(pcmd);
#if 0
    sprintf(mystr, "%d", ui->spinBox_1->value());
#else
    waittime=33;
    memset(mystr,0,4);
    if(pUtility->readfromfile("WAITSECS",mystr,SETTING_PATH))
    {
        waittime=atoi(mystr);
    }
    sprintf(mystr, "%d", waittime);
#endif
    sprintf(pcmd+cmdlen,"/waitsecs:%d ",waittime);
    cmdlen=strlen(pcmd);
    breakhotkey=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("BKHOTKEY",mystr,SETTING_PATH))
    {
        breakhotkey=atoi(mystr);
    }
    sprintf(pcmd+cmdlen,"/bkhotkey:%d ",breakhotkey);

    cmdlen=strlen(pcmd);


    if(0)
    {

        userinfo.setValue(QString("ConnectName0"),QString("Manual"));
        if(ui->targetcomboBox->currentText()!=QString("Manual"))
        {
            userinfo.setValue(QString("IPAddress0"),ui->targetcomboBox->currentText());
            qTmpstr=ui->targetcomboBox->currentText();
            qTmpstr.replace("\"","\\\"");


        }
        else
        {
            qTmpstr=userinfo.value("IPAddress0").toString();
            qTmpstr.replace("\"","\\\"");
        }

        userinfo.setValue(QString("Username0"),ui->username->text());
        userinfo.setValue(QString("Password0"),ui->password->text());
        if(pUtility->savetofile("host",NULL,"Manual",TMP_HOST_DATA))
        {

        }
    }
    else
    {
        //qTmpstr=ui->targetcomboBox->currentText();
        qTmpstr=xipaddr;
        //qTmpstr.replace("\\","\\\\");
        qTmpstr.replace("\"","\\\"");


    }
    sprintf(pcmd+cmdlen,"/v:\"%s\"",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);

    //usbfunction enable or disable
    readvalue=1;
    memset(mystr,0,4);
    if(pUtility->readfromfile("USBSTORAGE",mystr,USB_CONFIG_PATH))
    {
        readvalue=atoi(mystr);
    }
    if(readvalue==1)
    {
        //if usbfuction enable
        sprintf(pcmd+cmdlen," /drive:*,hotplug");
        cmdlen=strlen(pcmd);
    }
    //*****Check urbdrc is enabled or disabled*****
    readvalue=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("URBDRC",mystr,URBDRC_CONFIG_PATH)){
        readvalue=atoi(mystr);
    }

    if(readvalue==1){

        memset(cmdline,0,MAX_COMMAND_LENGTH);
        memset(ret,0,255);
        sprintf(cmdline,"cat /usr/ast/urbdrc_devices.conf");
        hopen=popen(cmdline,"r");
        while(fgets(ret, sizeof(ret) - 1, hopen) != NULL){}

        sprintf(pcmd+cmdlen," %s", ret);
        cmdlen=strlen(pcmd);
    }

    //*****Check smartcard is enabled or disabled*****

    readvalue=0;
    memset(mystr,0,4);
    if(pUtility->readfromfile("SMARTCARD",mystr,SMARTCARD_CONFIG_PATH)){
        readvalue=atoi(mystr);
    }

    if(readvalue==1){
        sprintf(pcmd+cmdlen," /smartcard:");
        cmdlen=strlen(pcmd);
    }

    //add for station name===========================
    memset(devname,0,128);
    if(!pUtility->readfromenv("devname", devname))
    {
        if(!pUtility->readfromfile("DEVICE_NAME", devname, SYSTEM_CONFIG_PATH))
        {
            //use PC Extender
            sprintf(devname,"PC Extender");
        }
    }

    qTmpstr=QString(devname);
    //qTmpstr.replace("\\","\\\\");
    qTmpstr.replace("\"","\\\"");
    qTmpstr.replace("\`","\\\`");
    sprintf(pcmd+cmdlen," /devname:\"%s\" ",qTmpstr.toLatin1().data());
    cmdlen=strlen(pcmd);
    //=============================================================

    rdpf = fopen(RDP_CONF_PATH, "w");
    if(rdpf)
    {
        fputs(pcmd, rdpf);
        fclose(rdpf);
        rdpf=0;
        system("sync");
    }

}

void MainWindow::on_sysinfo_clicked()
{
    mainDialog->setWindowTitle(tr("Administrative Setttings"));
    mainDialog->setParent(this);
    mainDialog->setStyleSheet("background-color:Pale gray;");
    mainDialog->setWindowFlags(Qt::FramelessWindowHint);
    mainDialog->setGeometry(187,184,700,432);
    mainDialog->show();
    mainDialog->mainUiInit();
    mainDialog->on_check_pressed();
}
