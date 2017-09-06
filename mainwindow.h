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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QEventLoop>
#include <QScrollArea>
#include <QTranslator>
#include "userconfig.h"
#include "advancedialog.h"
#include "dhcp_thread.h"
#include "waitingdialog.h"
#include "tooldialog.h"
#include "infodialog.h"
#include "utility.h"
#include "settingdialog.h"
#include <QPalette>
#include <QColor>
#include <QAction>
#include <ast_share.h>
#include <ioaccess.h>
#include "maindialog.h"
#define MAX_COMMANDLINK_LENGTH 1024
#define MAX_ADVANCE_TIMEOUT	50
#define MAX_HOST_LIST_NUM   10
#define MAX_STR_LEN         64
#define MAINWINDOW_HOSTNAME "Hostname / IP"
#define MAINWINDOW_USERNAME "Username"
#define MAINWINDOW_PASSWORD "Password"
//#define MAINWINDOW_AUTOLOGIN "Auto-Login"
//#define MAINWINDOW_REMEMBER_ME "Remember me"
//#define MAINWINDOW_CAPSLOCK "Caps Lock ON"
#define CUSTOMER_PATH		"/usr/ast/customer.conf"
#define TMP_HOST_DATA		"/usr/ast/tmphost"
enum {TAGEND,LABELSTART,DATASTART};
#define FIRST_WIFI_FILE "/tmp/firstwifi"
//=================================== 
#define CMDMAX 	MAX_COMMAND_LENGTH
#define CMDMCNT 16
#define CMD_SS  64
typedef struct customer_data{
    char flag;
	char cname[MAX_STR_LEN];
	char cpath[MAX_STR_LEN];
	unsigned int cgeo[4];
	unsigned int cfontsize;
}m_customer,*p_customer;
typedef struct hostinfo{
    char hostname[MAX_STR_LEN];
    char username[MAX_STR_LEN];
    char password[MAX_STR_LEN];
}m_hostinfo,*p_hostinfo;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
	int is_ip_exist(char* ipstr);	
	void add_managerip();
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int m_kb_fd;
    unsigned int m_KBD_status;
    QChar   m_char;
    int     m_rlefd;
    int     m_gpiofd;
    unsigned int  m_FWUpdate;
    int     m_bDataFromFlash;
    char *m_deskf;
    char m_cmdline[CMDMAX];
    char m_serverip[MAX_STR_LEN];
    char m_username[MAX_STR_LEN];
    char m_password[MAX_STR_LEN];
    char m_mac_addr[18];
    char m_ip_addr[16];
    char m_NetMask[16];
    char m_Gateway[16];
    char m_data_ver[20];
    int  m_bUseDHCP;
    int  m_crtidx;
    int  m_crtidx2;//20160816 add for dual monitor
    char m_background[256];
    char m_readwall[256];
    int  m_backgroundsize;
    char m_wallname[64];
    int  m_bBothCrt;
    int  m_frame_ack;
    int  m_bPreferMode;
    int	 m_multiMonitor;
	int  auto_resolution;
    QTimer *m_timer;
    int  m_hwver;

    void  parse_rdp_cmd(char*, int);
    char* QuerryIPbyHostName();
    void  UpdateNetInfo();
    void  TurnOnCRT(int crt_index);
    int     checkCRTstatus();
    int     checkNETbroken();  //1 broken
    void    getBoardInfo(unsigned int*);
    int     ParseConfigLine(char*);

	QTimer *crt_timer; 
	int show_last_connect();
	int 	hostindex;

	int accountflag;
	int advnace_idle_timeout;
    int advance_idle_counter;

    void init_dialog();
//    SettingDialog *settingwindow;
    WaitingDialog *waitwindow;
    MainDialog *mainDialog;
//    AdvanceDialog *advancewindow;
//    ToolDialog *toolwindow;
//    InfoDialog *infowindow;
    int getIPaddr(char* addr);
    QMessageBox *mainmsg;
    Utility *pUtility;
    int parse_vhost();
    int dislog_ctrl;
    int read_rememeber_me();
    int m_remember_me;
    bool manual_flag;
    
    m_hostinfo host_info[MAX_HOST_LIST_NUM];
    int combo_flag;
    void saveresolution(int size);
    int customer_init();
    int createlabel(p_customer pdata);
    int mode_check(PREFER_MODE* pmode);

    void setTranslator(QTranslator *translator, int language);
    int current_language;
    QTranslator *translator;
    QString current_hostip;
public slots:
	void initial_main();
    void onLoginfunc();

    int get_prefer_mode(PREFER_MODE* pmode);
    int get_prefer_mode_idx(int crtIdx, int &modeidx);
    int autoDetectMode(int b_multi);
    void timer_timeout();
    void crt_timeout();
    void onchange_dhcpmode(int mode);
    void onDHCPdone();
    void onDLGCTRL(int ctrl);
    void onDLGINFOFLAG();
    void onDLGWIFIFLAG();    
    void onDLGTOOLFLAG();
    void onDLGLOGFLAG();
    void onDLGLOGINFLAG();
    void onDLGADVANCEFLAG();
    void onDLGGENERALFLAG();

    void onTargetHighlighted(int index);
signals:
    void autodhcp();
    void NetInvalid();
    void ResDetect(int size);
private slots:
    void on_commandLinkButton_clicked(bool checked);
    void on_targetcomboBox_currentIndexChanged(int index);
	void on_configure_clicked();
    void on_targetcomboBox_currentIndexChanged(const QString &current_ConnectName);
    void Init_IP_Dialog();
    void changeLanguage(int language);
    void on_targetcomboBox_activated(int index);
    void on_WritetoRdpconf_Main(QString xipaddr, QString xusername, QString xpassword);

    void on_sysinfo_clicked();

private:
    Ui::MainWindow *ui;   
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H
