/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "William Lin" <william_lin@aspeedtech.com>
 *         "Chance Tsai" <chance_tsai@aspeedtech.com>
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
#include "advancedialog.h"
#include "ui_advancedialog.h"
#include <ioaccess.h>
#include <QFileInfo>
#include <QDebug>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <time.h>
static QSettings userinfo(userinfo_path,QSettings::IniFormat);
static QSettings usbconfig(USB_CONFIG_PATH,QSettings::IniFormat);
static QSettings urbdrc(URBDRC_CONFIG_PATH,QSettings::IniFormat);
static QSettings smartcard(SMARTCARD_CONFIG_PATH,QSettings::IniFormat);
static QSettings uiconf(NET_CONFIG_PATH,QSettings::IniFormat);
#define TMP_HOST_DATA		"/usr/ast/tmphost"
#define SETTINGS_SETTINGS_CONFIG_PATH		"/usr/ast/setting.conf"
#define CapsLock_ON    (1<<2)
#define NumLock_ON     (1<<1)
#define ScroLock_ON     (1<<0)
AdvanceDialog::AdvanceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvanceDialog)
{
    char cmdline[MAX_COMMAND_LEN];
    ui->setupUi(this);
    myProcess_network_update = new QProcess();
    advmsg = new QMessageBox;
    timer = new QTimer();
    advmsg->setParent(this);
    advmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    pUtility = new Utility;
    m_timer = new QTimer(this);
    logworker=NULL;
    logworkerthread=NULL;
    //add for remove account tab by flag
    account_flag=1;
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromenv("hwver",cmdline))
    {
        if(atoi(cmdline)==4)
        {
            account_flag=0;
        }
    }

    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromfile("REMEMBER_ME",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        if(1==atoi(cmdline))
            ui->RemembercheckBox->setChecked(true);
        else
            ui->RemembercheckBox->setChecked(false);
    }
    else
        ui->RemembercheckBox->setChecked(false);

    initial_string();
    initialdata();
    initial_account();
    initial_USB();
    initial_network();
    initPassword();
    adv_timer = new QTimer(this);
   // ui->account->setContextMenuPolicy(Qt::NoContextMenu);
    connect(timer, SIGNAL(timeout()), this, SLOT(hideMsg()));
    connect(ui->USBcheckBox,SIGNAL(stateChanged(int)),this,SLOT(USBcheckBox_clicked(int)));
    connect(ui->urbdrc_checkbox,SIGNAL(stateChanged(int)),this,SLOT(urbdrc_checkbox_clicked(int)));
    connect(&myProcess_lsusb, SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(showFinished_lsusb(int, QProcess::ExitStatus)));
    connect(myProcess_network_update, SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(showFinished_network_update(int,QProcess::ExitStatus)));
    connect(&myProcess_mac, SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(showFinishedMac(int, QProcess::ExitStatus)));
}
void AdvanceDialog::initial_FWUpdate()
{
    DIR *dir;
    FILE *hopen;

    dir = opendir(SAVE_ROOT_PATH);

    struct dirent *dent;
    struct stat st;
    char savepath[MAX_SAVE_PATH_LENGTH];
    char cmdline[MAX_COMMAND_LENGHT];
    char msgbuf[MAX_COMMAND_LEN];
    char sret[10];
    int index;
    int cnt_legalimg = 0;
    //********* Clean ComboBox *********
    ui->ImageFolderBox->clear();

    //********* Get USB dongle list *********
    if (dir){
        while ((dent = readdir(dir)) != NULL){

            if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
                continue;
            }

            //only display directory, but file
            memset(savepath,0,MAX_SAVE_PATH_LENGTH);
            sprintf(savepath,"%s%s",SAVE_ROOT_PATH,dent->d_name);

            if(stat(savepath, &st) == -1){
                PRINT_LOG((LOG_ERR,"stat error:%s\n",strerror(errno)));
                continue;
            }

            if(S_ISDIR(st.st_mode)){
                //add folder path to combobox
                ui->ImageFolderBox->addItem(savepath);
            }
        }
        closedir(dir);
    }
    else{
        PRINT_LOG((LOG_ERR,"opendir error:%s\n",strerror(errno)));
    }

    //********* Clean Combobox *********
    ui->ImageFileBox->clear();

    //********* Get *.bin file *********
    QByteArray folder_imagename = ui->ImageFolderBox->currentText().toLocal8Bit();
    if ((dir = opendir(folder_imagename.data())) != NULL){
        while ((dent = readdir (dir)) != NULL) {
            if(strstr(dent->d_name, ".bin") != NULL){
                //verify the *.bin is legal or not
                memset(cmdline,0,MAX_COMMAND_LENGHT);
                sprintf(cmdline,"hexdump -s 0x70000 %s/%s -v -n 4 | grep '7452 7452' -c", folder_imagename.data(), dent->d_name);
                qDebug()<<cmdline;
                hopen=popen(cmdline,"r");

                while(fgets(sret, sizeof(sret) - 1, hopen) != NULL){
                    //qDebug("%d", atoi(sret));
                }

                //List all image, but disable the illegal items
                if(atoi(sret) != 1){
                    ui->ImageFileBox->addItem(dent->d_name);
                    index = ui->ImageFileBox->findText(dent->d_name);
                    ui->ImageFileBox->setItemData(index, 0, Qt::UserRole - 1);
                }else{
                    ui->ImageFileBox->addItem(dent->d_name);
                    ui->ImageFileBox->setCurrentIndex(ui->ImageFileBox->findText(dent->d_name));
                    cnt_legalimg++;
                }
                pclose(hopen);
            }
        }
        //No legal image found, clean the combobox
        if(cnt_legalimg == 0){
            ui->ImageFileBox->clear();
            ui->ImageFileBox->addItem(ADVANCE_UPDATE_NO_VALID_BINARY);
            ui->ImageFileBox->setCurrentIndex(ui->ImageFileBox->findText(ADVANCE_UPDATE_NO_VALID_BINARY));
            ui->updatepushButton->setEnabled(false);
        }

        closedir(dir);
    }

    //********* If get dongle and least one legal *.bin, enable the btn *********
    if((cnt_legalimg != 0) && (ui->ImageFolderBox->count() == 1)){
        ui->updatepushButton->setEnabled(true);
    }else if(ui->ImageFolderBox->count() > 1){
        advmsg->setWindowTitle(tr("Warning"));
        advmsg->setText(tr("Please insert only 1 USB dongle!"));
        advmsg->exec();
        ui->updatepushButton->setEnabled(false);
        ui->ImageFolderBox->clear();
        ui->ImageFileBox->clear();
    }
    else{
        ui->updatepushButton->setEnabled(false);
    }
}


void AdvanceDialog::initial_USB()
{
    char cmdline[MAX_COMMAND_LENGHT];

    //Read Drive checkbox setting
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    if(pUtility->readfromfile("USBSTORAGE",cmdline,USB_CONFIG_PATH))
    {
        ui->USBcheckBox->setChecked(atoi(cmdline));
    }

    //Read Urbdrc checkbox setting
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    if(pUtility->readfromfile("URBDRC",cmdline,URBDRC_CONFIG_PATH))
    {
        qDebug()<<cmdline;
        ui->urbdrc_checkbox->setChecked(atoi(cmdline));
    }

}
void AdvanceDialog::initial_account()
{
    btnGroupAutologin = new QButtonGroup(this);
    btnGroupAutologin->addButton(ui->radioButton_1,0);
    btnGroupAutologin->addButton(ui->radioButton_2,1);
    btnGroupAutologin->addButton(ui->radioButton_3,2);
    btnGroupAutologin->addButton(ui->radioButton_4,3);
    btnGroupAutologin->addButton(ui->radioButton_5,4);
    btnGroupAutologin->addButton(ui->radioButton_6,5);
    btnGroupAutologin->addButton(ui->radioButton_7,6);
    btnGroupAutologin->addButton(ui->radioButton_8,7);

    connect(ui->radioButton_1, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_2, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_3, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_4, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_5, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_6, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_7, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));
    connect(ui->radioButton_8, SIGNAL(clicked()), this, SLOT(onRadioClickAutologin()));

    connect(ui->delete_01,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->delete_02,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->delete_03,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->delete_04,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->delete_05,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->delete_06,SIGNAL(clicked()),this,SLOT(DeleteCliecked()));
    connect(ui->edit_01,SIGNAL(clicked()),this,SLOT(editClicked()));
    connect(ui->edit_02,SIGNAL(clicked()),this,SLOT(editClicked()));
    connect(ui->edit_03,SIGNAL(clicked()),this,SLOT(editClicked()));
    connect(ui->edit_04,SIGNAL(clicked()),this,SLOT(editClicked()));
    connect(ui->edit_05,SIGNAL(clicked()),this,SLOT(editClicked()));
    connect(ui->edit_06,SIGNAL(clicked()),this,SLOT(editClicked()));
    FirstDlg_Init();
}
void AdvanceDialog::initial_network()
{
    FILE *fconf;
    QString macList;
    QString macARM;
    QString macOri;
    char cmdline[MAX_COMMAND_LENGHT];
    //read mac address
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    sprintf(cmdline,"ifconfig eth0 | awk '/HWaddr/ {print $5}'");
    fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
    	memset(cmdline,0,MAX_COMMAND_LENGHT);
        if(fgets(cmdline,20, fconf) != NULL)
        {   pclose(fconf);
            fconf=0;

            if(strlen(cmdline)>0)
            {
                cmdline[strlen(cmdline)-1]=0;
                macOri = uiconf.value("MACOrigen").toString();
                macARM = QString(cmdline).toUpper();
                qDebug()<<"macARM: "<<macARM;
                if(macARM==QString("00:1C:F0:01:01:01")){
                    macList = Random_Mac();
                    uiconf.setValue("MAC",macList);
                    //myProcess_mac.start("modify_ip.sh MAC " + macList);
                    memset(cmdline,0,MAX_COMMAND_LENGHT);
                    sprintf(cmdline,"modify_ip.sh MAC %s",macList.toLatin1().data());
                    qDebug()<<"cmdline: "<< QString(cmdline);
                    system(cmdline);
                    ui->MAC_addr->setText(macList);
                    uiconf.setValue("MAC",macList);
                }
                else
                {
                    ui->MAC_addr->setText(QString(cmdline));
                    uiconf.setValue("MAC",cmdline);
                }
            }
        }
    }   	 
    //read uiconf.conf to check use DHPC or not   
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    if(readfromfile("USE_DHCP",cmdline,NET_CONFIG_PATH))
    {
        net_mode=atoi(cmdline);
        if(net_mode == DHCP_MODE)
        {
            //read etho0 ip address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            sprintf(cmdline,"ifconfig eth0 | awk '/inet addr/{print substr($2,6)}'");
            fconf=popen(cmdline,"r");
            if(fconf!=NULL)
            {
                memset(cmdline,0,MAX_COMMAND_LENGHT);
                ui->DHCP_IP->setText(tr("No Network!"));
            if(fgets(cmdline,20, fconf) != NULL)
            {
                if(strlen(cmdline)>0)
                {
                cmdline[strlen(cmdline)-1]=0;
                ui->DHCP_IP->setText(QString(cmdline));
                }
            }


                pclose(fconf);
                fconf=0;
            }
            else{
                //syslog
                PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));

            }
            ui->R_UseDHCP->setChecked(true);
            ui->R_FixIP->setChecked(false);
            on_R_UseDHCP_clicked(true);
            ui->FIXIP->clear();
            ui->NetMask->clear();
            ui->GateWay->clear();
            ui->DNS1->clear();
            ui->DNS2->clear();
        }else //use fix ip
        {
            ui->DHCP_IP->clear();
            ui->R_UseDHCP->setChecked(false);
            ui->R_FixIP->setChecked(true);
            ui->B_RefreshNet->setEnabled(false);

            //read fix ip address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("IP_ADDR",cmdline,NET_CONFIG_PATH))
            {
                fixipstr=QString(cmdline);
                ui->FIXIP->setText(QString(cmdline));
            }
            //read fix netmask address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("NETMASK",cmdline,NET_CONFIG_PATH))
            {
                netmaskstr=QString(cmdline);
                ui->NetMask->setText(QString(cmdline));
            }
            //read fix gateway address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("GATEWAY",cmdline,NET_CONFIG_PATH))
            {
                gatewaystr=QString(cmdline);
                ui->GateWay->setText(QString(cmdline));
            }
            //read DNS1 address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("DNS1",cmdline,NET_CONFIG_PATH))
            {
                DNS1str=QString(cmdline);
                ui->DNS1->setText(QString(cmdline));
            }
            //read DNS1 address
            memset(cmdline,0,MAX_COMMAND_LENGHT);
            if(readfromfile("DNS2",cmdline,NET_CONFIG_PATH))
            {
                DNS2str=QString(cmdline);
                ui->DNS2->setText(QString(cmdline));
            }
        }
    }    

}
void AdvanceDialog::initial_string()
{
}
void AdvanceDialog::initialdata()
{	

}
void AdvanceDialog::onDHCPdone()
{	
    initial_network();  
    emit change_dhcpmode(1);  
}
void AdvanceDialog::onautodhcp()
{
	initial_network();
}
void AdvanceDialog::onNetInvalid()
{
    ui->DHCP_IP->setText(tr("No Network!"));
}

void AdvanceDialog::showEvent(QShowEvent * event)
{
    initialdata();	
    QDialog::showEvent(event);
}
AdvanceDialog::~AdvanceDialog()
{
    delete ui;
}
void AdvanceDialog::on_recoveryButton_clicked()
{
    FILE *fconf;
    int reboot_flag=0;
    char cmdline[MAX_COMMAND_LENGHT];	
    //recovery account to default value
    
    if(ui->settingscheckBox->isChecked())
    {
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        sprintf(cmdline,"cp %s %s",DEFAULT_SETTINGS_CONFIG_PATH,SETTINGS_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL)
        {
        pclose(fconf);
        fconf=0;
        system("sync");
        }
        else
        {
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }
    }
    
    //recovery account to default value
    if(ui->accountcheckBox->isChecked())
    {
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        //sprintf(cmdline,"awk -F '=' '/USERNAME/ {print $2}' %s",UI_CONFIG_PATH);
        sprintf(cmdline,"cp %s %s",DEFAULT_ACCOUNT_CONFIG_PATH,ACCOUNT_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL)
        {
        pclose(fconf);
        fconf=0;
        system("sync");
        }
        else
        {
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }
    }
    if(ui->networkcheckBox->isChecked())
    {

		//recovery uiconf.conf
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"cp %s %s",DEFAULT_NET_CONFIG_PATH,NET_CONFIG_PATH);
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
		pclose(fconf);
		fconf=0;
		system("sync");
		}
		else
		{
		    //syslog
		    PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
		}
    	if(ui->R_FixIP->isChecked())
    	{		
		//Restart needed for Network recover to take effect.
		reboot_flag=1;
        }
    }
    //usb config load default
    if(ui->usbrestorecheckBox->isChecked())
    {
        //recovery usb storage settings
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        sprintf(cmdline,"cp %s %s",DEFAULT_USB_CONFIG_PATH,USB_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL){
            pclose(fconf);
            fconf=0;
            system("sync");
        }else{
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }

        //recovery smartcard redirection settings
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        sprintf(cmdline,"cp %s %s", DEFAULT_SMARTCARD_CONFIG_PATH, SMARTCARD_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL){
            pclose(fconf);
            fconf=0;
            system("sync");
        }else{
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }

        //recovery other USB redirection settings
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        sprintf(cmdline,"cp %s %s", DEFAULT_URBDRC_CONFIG_PATH, URBDRC_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL){
            pclose(fconf);
            fconf=0;
            system("sync");
        }else{
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }
    }
    //device name load default
    if(ui->devicenamecheckBox->isChecked())
    {
        //recovery uiconf.conf
        memset(cmdline,0,MAX_COMMAND_LENGHT);
        sprintf(cmdline,"cp %s %s",DEFAULT_SYSTEM_CONFIG_PATH,SYSTEM_CONFIG_PATH);
        fconf=popen(cmdline,"r");
        if(fconf!=NULL)
        {
        pclose(fconf);
        fconf=0;
        system("sync");
        }
        else
        {
            //syslog
            PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));
        }
    }
    //device name load default
    if(ui->password->isChecked())
    {
        system("rm /usr/ast/password.conf");
    }
    //all data on ui load default
    emit loaddefault();
    //initialdata();    

    advmsg->setText(tr("Recovery completed."));
    if(!reboot_flag)
    {     
         advmsg->setText(tr("Recovery completed."));
         advmsg->exec();
     }
    else
    {//Restart needed for Network recover to take effect.
        advmsg->setText(tr("Restart needed for Network recover to take effect."));
        advmsg->exec();
        system("reboot");
    }
}

void AdvanceDialog::on_B_RefreshNet_clicked()
{
    char cmdline[MAX_COMMAND_LEN];
    if(check_link())
    {
        uiconf.setValue("USE_DHCP",DHCP_MODE);
	    waitwindow = new WaitingDialog(this);
	    waitwindow->setFixedSize(350,79);
        waitwindow->setWindowTitle(tr("DHCP Renewing......"));
	    waitwindow->setParent(this);
	    waitwindow->setStyleSheet("background-color:Pale gray;");
	    waitwindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
	    //20160225 modify for dhcp renew
		memset(cmdline,0,MAX_COMMAND_LEN);
		sprintf(cmdline,"udhcpc -i eth0 -t 3 -T 3 -q -n");
		waitwindow->setcmdline(cmdline);	    
	    connect(waitwindow, SIGNAL(dhcprenewdone()),SLOT(onDHCPdone()));
	    waitwindow->exec();

    }
    else
    {
         ui->DHCP_IP->setText(tr("No Network!"));
    }
}
QString AdvanceDialog::Random_Mac()
{
    unsigned int time_value;
    time_value = time(0);
    QString random_num;
    QString stringMac = "00:1C:F0:";
    char cmdline[MAX_COMMAND_LENGHT];
    memset(cmdline,0,MAX_COMMAND_LENGHT);
    sprintf(cmdline,"io 0 1e6e2078");
    FILE *fconf=popen(cmdline,"r");
    if(fconf!=NULL)
    {
        if(fgets(cmdline,20, fconf) != NULL)
        {
            if(strlen(cmdline)>0)
            {
                cmdline[strlen(cmdline)-1]=0;
                random_num = QString(cmdline).left(6);
                random_num.insert(2, QString(":"));
                random_num.insert(5, QString(":"));
                stringMac = stringMac + random_num.toUpper();

            }
        }
        else
           stringMac = QString("00:1C:F0:01:01:01");
        pclose(fconf);

    }
    else
       stringMac = QString("00:1C:F0:01:01:01");
    fconf=0;

//    qsrand(time_value);

//    int number_01,number_02,number_03;
//    number_01 = qrand()%255;
//    qsrand(time_value - number_01);
//    number_02 = qrand()%255;
//    qsrand(time_value - number_02);
//    number_03 = qrand()%255;

//    QByteArray ba1,ba2,ba3;
//    stringMac = stringMac + ba1.append(number_01).toHex().toUpper() + ":";
//    stringMac = stringMac + ba2.append(number_02).toHex().toUpper() + ":";
//    stringMac = stringMac + ba3.append(number_03).toHex().toUpper();
    return stringMac;
}
void AdvanceDialog::on_NET_Submit_Button_clicked()
{
    FILE *fconf;
    QString newstr;
    char dhcpdata1[2];
    char dhcpdata2[2];
    char cmdline[MAX_COMMAND_LEN];	
    QString macList = ui->MAC_addr->text();
    //Random_Mac();
    if(macList.length() == 17){
        if(macList != uiconf.value("MAC").toString()){
            uiconf.setValue("MAC",macList);
            myProcess_mac.start("modify_ip.sh MAC " + macList);

        }
    }

    if(ui->R_UseDHCP->isChecked())
    { //save to uiconf.conf use DHCP
		memset(dhcpdata1,0,2);
		memset(dhcpdata2,0,2);
		if(pUtility->readfromfile("USE_DHCP",dhcpdata1,NET_CONFIG_PATH))
		{
		}
		sprintf(dhcpdata2,"1");
		if(pUtility->savetofile("USE_DHCP",dhcpdata1,dhcpdata2,NET_CONFIG_PATH))
		{
			net_mode=DHCP_MODE;
			ui->B_RefreshNet->setEnabled(true);		
			//qDebug() << "save USE_DHCP=1 OK";
		}
		on_B_RefreshNet_clicked();		
        //execute DHCP to get dynamic ip address        
    }
    else if(ui->R_FixIP->isChecked())
    { 
      //no matter what netmode in uiconf.conf, if user set fix ip mode, then save/set it.
      //because user maybe just only change ip address/netmask/gateway

      advmsg->setWindowTitle(tr("Warning"));

      //check address valid
      if(pUtility->isValidIP(ui->FIXIP->text())!=AF_INET)
      {
      	      //Invalid IP address, popup message, and set focus

          advmsg->setText(tr("Invalid IP address! Please try again."));
		if(advmsg->exec()==QMessageBox::Ok)
		{
			ui->FIXIP->setFocus();
		}
		return;
      }
      /*(if(!pUtility->isValidMask(ui->NetMask->text().toLatin1().data()))
      {
        qDebug()<<pUtility->isValidIP(ui->NetMask->text());
        //Invalid netmask, popup message, and set focus
        advmsg->setText(tr("Invalid Netmask! Please try again."));
		if(advmsg->exec()==QMessageBox::Ok)
		{      	      
			ui->NetMask->setFocus();
		}
        return;
      }*/
      if(pUtility->isValidIP(ui->GateWay->text())!=AF_INET)
      {
      	      //Invalid gateway, popup message, and set focus		
          advmsg->setText(tr("Invalid Gateway! Please try again."));
		if(advmsg->exec()==QMessageBox::Ok)
		{        	      
			ui->GateWay->setFocus();
      	}
      	      return;      	      
      } 
      //have to check empty or not
      if((ui->DNS1->text().length() !=3 ) && (pUtility->isValidIP(ui->DNS1->text())!=AF_INET))
      {

        advmsg->setText(tr("Invalid DNS1! Please try again."));
		if(advmsg->exec()==QMessageBox::Ok)
		{        	      
			ui->DNS1->setFocus();
      	}
      	      return;      	      
      }  
      //have to check empty or not
      if((ui->DNS2->text().length() !=3 ) && (pUtility->isValidIP(ui->DNS2->text())!=AF_INET))
      {
              //Invalid gateway, popup message, and set focus
        advmsg->setText(tr("Invalid DNS2! Please try again."));
		if(advmsg->exec()==QMessageBox::Ok)
		{        	      
			ui->DNS2->setFocus();
      	}
      	      return;      	      
      }       
      //save to uiconf.conf use fixip
	memset(cmdline,0,MAX_COMMAND_LENGHT);
	memset(dhcpdata1,0,2);
	memset(dhcpdata2,0,2);
		if(pUtility->readfromfile("USE_DHCP",dhcpdata1,NET_CONFIG_PATH))
		{
		}
	strcpy(dhcpdata2,"0");
	if(pUtility->savetofile("USE_DHCP",dhcpdata1,dhcpdata2,NET_CONFIG_PATH))
	{				
        ui->DHCP_IP->clear();
		//save fix ip
		newstr=ui->FIXIP->text();
		if(pUtility->savetofile("IP_ADDR",fixipstr.toLatin1().data(),newstr.toLatin1().data(),NET_CONFIG_PATH))
		{
			//qDebug()<< "save fix ip ok";	
		}
		//save netmask
		newstr=ui->NetMask->text();
		if(pUtility->savetofile("NETMASK",netmaskstr.toLatin1().data(),newstr.toLatin1().data(),NET_CONFIG_PATH))
		{
			//qDebug()<< "save netmask ok";
		}
		//save gateway
		newstr=ui->GateWay->text();
        if(pUtility->savetofile("GATEWAY",gatewaystr.toLatin1().data(),newstr.toLatin1().data(),NET_CONFIG_PATH))
		{
			//qDebug()<< "save gateway ok"; 	
		}
		//save DNS1
		if(ui->DNS1->text().length()==3)
		{				
	        newstr="";
		}
		else
		{
			newstr=ui->DNS1->text();	
		}
		if(pUtility->savetofile("DNS1",DNS1str.toLatin1().data(),newstr.toLatin1().data(),NET_CONFIG_PATH))
		{
			//qDebug()<< "save gateway ok"; 	
		}	
		//save DNS2
		if(ui->DNS2->text().length()==3)
		{				
	        newstr="";
		}
		else
		{
			newstr=ui->DNS2->text();	
		}
		if(pUtility->savetofile("DNS2",DNS2str.toLatin1().data(),newstr.toLatin1().data(),NET_CONFIG_PATH))
		{
			//qDebug()<< "save gateway ok"; 	
		}		
		//use ifconfig to set ip and netmask	
		net_mode=FIXIP_MODE;
		fixipstr=ui->FIXIP->text();
		netmaskstr=ui->NetMask->text();
		gatewaystr=ui->GateWay->text();
		if(ui->DNS1->text().length()==3)
		{
				DNS1str="";
		}
		else
		{
				DNS1str=ui->DNS1->text();
		}
		if(ui->DNS2->text().length()==3)
		{
				DNS2str="";
		}
		else
		{
				DNS2str=ui->DNS2->text();
		}
		//set ip address
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"ifconfig eth0 %s",fixipstr.toLatin1().data());
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			pclose(fconf);
			fconf=0;
		}   
		else
		{
			//syslog   
			PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));	
		}
		//set netmask
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"ifconfig eth0 netmask %s",netmaskstr.toLatin1().data());
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			pclose(fconf);
			fconf=0;
		}   
		else
		{
			//syslog   
			PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));	
		}		
		//use route to add default gateway
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"route del default");
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			pclose(fconf);
			fconf=0;
		}   
		else
		{
			//syslog   
			PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));	
		}
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"route add default gw %s",gatewaystr.toLatin1().data());
		fconf=popen(cmdline,"r");
		if(fconf!=NULL)
		{
			pclose(fconf);
			fconf=0;
		}   
		else
		{
			//syslog   
			PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));	
		}	
		//modify nameserver in /etc/resolv.conf
		//first clear /etc/resolv.conf
		system("cp /dev/null /etc/resolv.conf");
		//if DNS1 exist
		if(DNS1str.length()!=0)
		{
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"echo 'nameserver %s' >> /etc/resolv.conf",DNS1str.toLatin1().data());
		system(cmdline);
		}
		//if DNS2 exist
		if(DNS2str.length()!=0)
		{		
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		sprintf(cmdline,"echo 'nameserver %s' >> /etc/resolv.conf",DNS2str.toLatin1().data());
		system(cmdline);		
		}
		emit change_dhcpmode(0);

        advmsg->setWindowTitle(tr("Information"));
        advmsg->setText(tr("Change to Fixed IP mode completed."));
		advmsg->exec();			
	}
    }//fixed ip
}
void AdvanceDialog::on_R_UseDHCP_clicked(bool checked)
{
    if(checked)
    {
        ui->FIXIP->setEnabled(false);
        ui->NetMask->setEnabled(false);
        ui->GateWay->setEnabled(false);
        ui->DNS1->setEnabled(false);
        ui->DNS2->setEnabled(false);
        ui->B_RefreshNet->setEnabled(true);
        if(net_mode==DHCP_MODE)
        {
        	ui->B_RefreshNet->setEnabled(true);
        }
    }
}

void AdvanceDialog::on_R_FixIP_clicked(bool checked)
{
    if(checked)
    {
        ui->FIXIP->setEnabled(true);
        ui->NetMask->setEnabled(true);
        ui->GateWay->setEnabled(true);
        ui->DNS1->setEnabled(true);
        ui->DNS2->setEnabled(true);
        ui->B_RefreshNet->setEnabled(false);
    }
}
void AdvanceDialog::on_advanceTab_currentChanged(int index)
{
    if(!account_flag)
    {
    	index++;
    }
    switch(index)
    {
        case 0: //account
            ui->stacked_advanced->setCurrentIndex(0);break;
        case 1: //network
            ui->stacked_advanced->setCurrentIndex(1);break;
        case 2: //usb
            ui->stacked_advanced->setCurrentIndex(2);break;
        case 3: //recovery
            ui->stacked_advanced->setCurrentIndex(3);break;
        case 4://password
            ui->stacked_advanced->setCurrentIndex(4);break;
        default:
                break;
    }
}
int AdvanceDialog::savetofile(const char* keyitem,char* olddata,char* newdata,const char* path)
{
	FILE *fconf;
	int ret=0;
	char cmdline[MAX_COMMAND_LENGHT];		
	memset(cmdline,0,MAX_COMMAND_LENGHT);
	//net_mode <== network mode in uiconf.conf
	sprintf(cmdline,"sed -i 's/%s=%s/%s=%s/g' %s",keyitem,olddata,keyitem,newdata,path);
	fconf=popen(cmdline,"r");
	if(fconf!=NULL)
	{
		pclose(fconf);
		fconf=0;
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		if(readfromfile(keyitem,cmdline,path))
		{
			if(!strcmp(newdata,cmdline))
			{				
				ret=1;
			}
			else
			{
				PRINT_LOG((LOG_ERR,"Save %s to %s fail!\n",keyitem,path));
			}
		}
		else
		{
		}
	}   
	else
	{
		//syslog   
		PRINT_LOG((LOG_ERR,"popen error:%s\n",strerror(errno)));	
	}  	
	return ret;
}
int AdvanceDialog::readfromfile(const char* keyitem,char* value,const char* path)
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
int AdvanceDialog::check_link()
{
	int ret=0;
	FILE *fd;
	char cmdline[MAX_COMMAND_LENGHT];
	memset(cmdline,0,MAX_COMMAND_LENGHT);
	sprintf(cmdline,"cat /sys/class/net/eth0/carrier");
	fd=popen(cmdline,"r");
	if(fd!=NULL)
	{
		memset(cmdline,0,MAX_COMMAND_LENGHT);
		if(fgets(cmdline,20, fd) != NULL)
		{
		    if(strlen(cmdline)>0)
		    {				    
			cmdline[strlen(cmdline)-1]=0;	
			ret=atoi(cmdline);
		    }
		}
		pclose(fd);
		fd=0;
	}
    return ret;	
}
void AdvanceDialog::on_updatepushButton_clicked()
{
    char updatecmdline[256];
    char cmdline[MAX_COMMAND_LENGHT];	
    FILE *pfd;

    advmsg->setWindowTitle(tr("Warning"));
	//add for confirm to update
	advmsg->setStandardButtons(QMessageBox::Yes|QMessageBox::No);
	advmsg->setDefaultButton(QMessageBox::No);
	memset(cmdline,0,MAX_COMMAND_LEN);
    advmsg->setText(tr("Are you sure to update?"));

	if(advmsg->exec() == QMessageBox::No)
	{
        return;
	}
	//restore button
    if(ui->ImageFolderBox->currentText().isEmpty() || ui->ImageFileBox->currentText().isEmpty()){
        advmsg->setText(tr("WARNING! No update image file"));
        advmsg->exec();
    }else{
        advmsg->setStandardButtons(QMessageBox::Ok);
        advmsg->setDefaultButton(QMessageBox::Ok);
        //=================================
        advmsg->setText(tr("WARNING! Going to update firmware. Please do NOT power off or shut down the device before the update is completed."));

        advmsg->exec();

        QByteArray folder_image = ui->ImageFolderBox->currentText().toLocal8Bit();
        QByteArray folder_imagename = ui->ImageFileBox->currentText().toLocal8Bit();

//        sprintf(updatecmdline,"AST_UpdateFW -c 1 -s %s/%s %s", folder_image.data(), folder_imagename.data(),"-t 0,2,3,5,6,4");
//        myProcess_network_update->start(updatecmdline);
//        updatewindow = new UpdateDialog(this);
//        updatewindow->setData(folder_imagename.data());
//        system("fw_setenv safe_mode 1");
//        updatewindow->setWindowTitle(tr("Firmware Updating..."));
//        updatewindow->setParent(this);
//        updatewindow->setFixedSize(384,91);
//        updatewindow->setStyleSheet("background-color:Pale gray;");
//        updatewindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
//        updatewindow->exec();

        sprintf(cmdline, "killall power; killall manager; killall killer; killall iowakeup");
        system(cmdline);

        sprintf(updatecmdline,"AST_UpdateFW -c 5274 -s %s/%s &", folder_image.data(), folder_imagename.data());
        system(updatecmdline);

        QApplication::quit();

    }
}

void AdvanceDialog::on_ImageFolderBox_currentIndexChanged()
{
    DIR *dir;
    struct dirent *dent;
    //Clean ui->ImageFileBox
    ui->ImageFileBox->clear();
    //Get *.bin file
    QByteArray folder_imagename = ui->ImageFolderBox->currentText().toLocal8Bit();
    if ((dir = opendir(folder_imagename.data())) != NULL){
        while ((dent = readdir (dir)) != NULL) {
            if(strstr(dent->d_name, ".bin") != NULL){
                ui->ImageFileBox->addItem(dent->d_name);
            }
        }
        closedir(dir);
    }
}
void AdvanceDialog::on_refresh_USB_Btn_clicked()
{
    initial_FWUpdate();
}

int AdvanceDialog::checkUSB()
{

}
void AdvanceDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Escape)
    {
        AdvanceDialog::accept();
        return;
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}    

void AdvanceDialog::on_New_clicked()
{
    if(userinfo.value("UserNumber").toInt() >= 7){
        advmsg->setText(tr("Userinfo list if full!"));
        advmsg->exec();
        return;
    }
    createUsers = new Create_Users(this);
    createUsers->exec();
    FirstDlg_Init();
}

void AdvanceDialog::DeleteCliecked()
{
    QString before,after;
    QPushButton* pt_poweron=qobject_cast<QPushButton*>(sender());
    QStringList myOptions;
    int current_select_obj;
    int current_UserNumber = userinfo.value("UserNumber").toInt();
    //通过获取objectName来判断用户单击的是哪个按钮,通过switch把操作指令发送出去
    myOptions << "delete_01" << "delete_02" << "delete_03" << "delete_04" << "delete_05" << "delete_06";
    current_select_obj = myOptions.indexOf(pt_poweron->objectName()) + 1;
    int j = current_select_obj;

    qDebug()<<"current_select_obj: "<<current_select_obj;

    if(j==userinfo.value("AutoLoginnum").toInt())
    {
        userinfo.setValue("AutoLoginnum","8");
        userinfo.setValue("ContinueLogin","0");
    }

    before = QString::number(current_select_obj,10);
    after = QString::number(current_select_obj+1,10);
    for(int i = 0;i < 6-j;i++){
        userinfo.setValue("ConnectName"+before,userinfo.value("ConnectName"+after).toString());
        userinfo.setValue("IPAddress"+before,userinfo.value("IPAddress"+after).toString());
        userinfo.setValue("Username"+before,userinfo.value("Username"+after).toString());
        userinfo.setValue("Password"+before,userinfo.value("Password"+after).toString());
        current_select_obj = current_select_obj + 1;
        before = QString::number(current_select_obj,10);
        after = QString::number(current_select_obj+1,10);
    }
        userinfo.setValue("ConnectName"+before,"");
        userinfo.setValue("IPAddress"+before,"");
        userinfo.setValue("Username"+before,"");
        userinfo.setValue("Password"+before,"");
       // settings.setValue("option"+s,"");
        //settings.setValue("autoConnect"+s,"");
        current_UserNumber = current_UserNumber - 1;
        userinfo.setValue("UserNumber",QString::number(current_UserNumber,10));
        FirstDlg_Init();
}

void AdvanceDialog::editClicked()
{
    QString before,after;
    QPushButton* pt_poweron=qobject_cast<QPushButton*>(sender());
    QStringList myOptions;
    int currentSelectObj;
    //通过获取objectName来判断用户单击的是哪个按钮,通过switch把操作指令发送出去
    myOptions << "edit_01" << "edit_02" << "edit_03" << "edit_04" << "edit_05" << "edit_06";
    currentSelectObj = myOptions.indexOf(pt_poweron->objectName()) + 1;
    before = QString::number(currentSelectObj,10);
    after = QString::number(currentSelectObj+1,10);

    createUsers = new Create_Users(this);
    createUsers->editUsers(userinfo.value("ConnectName" + before).toString(),userinfo.value("IPAddress" + before).toString(),userinfo.value("Username" + before).toString(),userinfo.value("Password" + before).toString(),currentSelectObj);
    createUsers->exec();
    emit WritetoRdpconf(userinfo.value("IPAddress"+before).toString(), userinfo.value("Username"+before).toString(), userinfo.value("Password"+before).toString());
    FirstDlg_Init();
}

void AdvanceDialog::FirstDlg_Init(){
    ui->delete_01->setVisible(false);
    ui->delete_02->setVisible(false);
    ui->delete_03->setVisible(false);
    ui->delete_04->setVisible(false);
    ui->delete_05->setVisible(false);
    ui->delete_06->setVisible(false);

    ui->edit_01->setVisible(false);
    ui->edit_02->setVisible(false);
    ui->edit_03->setVisible(false);
    ui->edit_04->setVisible(false);
    ui->edit_05->setVisible(false);
    ui->edit_06->setVisible(false);

    ui->user01->clear();
    ui->user02->clear();
    ui->user03->clear();
    ui->user04->clear();
    ui->user05->clear();
    ui->user06->clear();

    ui->radioButton_1->setVisible(false);
    ui->radioButton_2->setVisible(false);
    ui->radioButton_3->setVisible(false);
    ui->radioButton_4->setVisible(false);
    ui->radioButton_5->setVisible(false);
    ui->radioButton_6->setVisible(false);

    ui->ContinuecheckBox_1->setVisible(false);
    ui->ContinuecheckBox_2->setVisible(false);
    ui->ContinuecheckBox_3->setVisible(false);
    ui->ContinuecheckBox_4->setVisible(false);
    ui->ContinuecheckBox_5->setVisible(false);
    ui->ContinuecheckBox_6->setVisible(false);

    ui->radioButton_7->setVisible(false);
    ui->ContinuecheckBox_7->setVisible(false);
    ui->user07->setVisible(false);
    ui->delete_07->setVisible(false);
    ui->edit_07->setVisible(false);

    QString str_exist = QString(tr("Yes"));
    QString str_empty = QString(tr("N/A"));
    QString str_name;
    QString str_passwd;


    int NUM=userinfo.value("UserNumber").toInt();
    for(int i=1 ;i< NUM; i++){
        switch(i){
            case 1:
                ui->delete_01->setVisible(true);
                ui->edit_01->setVisible(true);
                ui->radioButton_1->setVisible(true);
                ui->ContinuecheckBox_1->setVisible(true);

                if(userinfo.value("Username1").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password1").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;

                ui->user01->setText(userinfo.value("ConnectName1").toString() \
                                    + "   " + userinfo.value("IPAddress1").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);

                break;
            case 2:
                ui->delete_02->setVisible(true);
                ui->edit_02->setVisible(true);
                ui->radioButton_2->setVisible(true);
                ui->ContinuecheckBox_2->setVisible(true);
                if(userinfo.value("Username2").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password2").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;
                ui->user02->setText(userinfo.value("ConnectName2").toString() \
                                    + "   " + userinfo.value("IPAddress2").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);
                break;
            case 3:
                ui->delete_03->setVisible(true);
                ui->edit_03->setVisible(true);
                ui->radioButton_3->setVisible(true);
                ui->ContinuecheckBox_3->setVisible(true);
                if(userinfo.value("Username3").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password3").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;
                ui->user03->setText(userinfo.value("ConnectName3").toString() \
                                    + "   " + userinfo.value("IPAddress3").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);
                break;
            case 4:
                ui->delete_04->setVisible(true);
                ui->edit_04->setVisible(true);
                ui->radioButton_4->setVisible(true);
                ui->ContinuecheckBox_4->setVisible(true);
                if(userinfo.value("Username4").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password4").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;
                ui->user04->setText(userinfo.value("ConnectName4").toString() \
                                    + "   " + userinfo.value("IPAddress4").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);
                break;
            case 5:
                ui->delete_05->setVisible(true);
                ui->edit_05->setVisible(true);
                ui->radioButton_5->setVisible(true);
                ui->ContinuecheckBox_5->setVisible(true);
                if(userinfo.value("Username5").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password5").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;
                ui->user05->setText(userinfo.value("ConnectName5").toString() \
                                    + "   " + userinfo.value("IPAddress5").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);
               break;
            case 6:
                ui->delete_06->setVisible(true);
                ui->edit_06->setVisible(true);
                ui->radioButton_6->setVisible(true);
                ui->ContinuecheckBox_6->setVisible(true);
                if(userinfo.value("Username6").toString()=="")
                    str_name = str_empty;
                else
                    str_name = str_exist;

                if(userinfo.value("Password6").toString()=="")
                    str_passwd = str_empty;
                else
                    str_passwd = str_exist;
                ui->user06->setText(userinfo.value("ConnectName6").toString() \
                                    + "   " + userinfo.value("IPAddress6").toString() \
                                    + "   " + str_name \
                                    + "   " + str_passwd);
                break;
        }
    }

    if(userinfo.value("IPAddress0").toString()!="")
    {
        ui->radioButton_7->setVisible(true);
        ui->ContinuecheckBox_7->setVisible(true);
        ui->user07->setVisible(true);
        ui->delete_07->setVisible(true);
//        ui->edit_07->setVisible(false);
        if(userinfo.value("Username0").toString()=="")
            str_name = str_empty;
        else
            str_name = str_exist;

        if(userinfo.value("Password0").toString()=="")
            str_passwd = str_empty;
        else
            str_passwd = str_exist;
        ui->user07->setText(userinfo.value("ConnectName0").toString() \
                            + "   " + userinfo.value("IPAddress0").toString() \
                            + "   " + str_name \
                            + "   " + str_passwd);
    }

    int autol=userinfo.value("AutoLoginnum").toInt();
    int continuel=userinfo.value("ContinueLogin").toInt();

    qDebug()<<"AutoLoginnum: "<<autol;
    qDebug()<<"ContinueLogin: "<<continuel;

    ui->ContinuecheckBox_1->setCheckable(true);
    ui->ContinuecheckBox_2->setCheckable(true);
    ui->ContinuecheckBox_3->setCheckable(true);
    ui->ContinuecheckBox_4->setCheckable(true);
    ui->ContinuecheckBox_5->setCheckable(true);
    ui->ContinuecheckBox_6->setCheckable(true);
    ui->ContinuecheckBox_7->setCheckable(true);

    ui->ContinuecheckBox_1->setChecked(false);
    ui->ContinuecheckBox_2->setChecked(false);
    ui->ContinuecheckBox_3->setChecked(false);
    ui->ContinuecheckBox_4->setChecked(false);
    ui->ContinuecheckBox_5->setChecked(false);
    ui->ContinuecheckBox_6->setChecked(false);
    ui->ContinuecheckBox_7->setChecked(false);

    ui->ContinuecheckBox_1->setCheckable(false);
    ui->ContinuecheckBox_2->setCheckable(false);
    ui->ContinuecheckBox_3->setCheckable(false);
    ui->ContinuecheckBox_4->setCheckable(false);
    ui->ContinuecheckBox_5->setCheckable(false);
    ui->ContinuecheckBox_6->setCheckable(false);
    ui->ContinuecheckBox_7->setCheckable(false);

    if((autol>=NUM)&&(autol!=8))
    {
        autol=8;
        userinfo.setValue("AutoLoginnum","8");
        userinfo.setValue("ContinueLogin","0");

    }

    switch(autol){
        case 1:
            ui->radioButton_1->setChecked(true);
            ui->ContinuecheckBox_1->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_1->setChecked(true);
            break;
        case 2:
            ui->radioButton_2->setChecked(true);
            ui->ContinuecheckBox_2->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_2->setChecked(true);
            break;
        case 3:
            ui->radioButton_3->setChecked(true);
            ui->ContinuecheckBox_3->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_3->setChecked(true);
            break;
        case 4:
            ui->radioButton_4->setChecked(true);
            ui->ContinuecheckBox_4->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_4->setChecked(true);
            break;
        case 5:
            ui->radioButton_5->setChecked(true);
            ui->ContinuecheckBox_5->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_5->setChecked(true);
            break;
        case 6:
            ui->radioButton_6->setChecked(true);
            ui->ContinuecheckBox_6->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_6->setChecked(true);
            break;
        case 7:
            ui->radioButton_7->setChecked(true);
            ui->ContinuecheckBox_7->setCheckable(true);
            if(continuel==1)
                ui->ContinuecheckBox_7->setChecked(true);
            break;
        case 8:
            ui->radioButton_8->setChecked(true);
            break;
    }

    if(ui->radioButton_8->isChecked())
        update_AutoLogin(false);
    else
        update_AutoLogin(true);

    //ui->ContinuecheckBox_1->setVisible(false);
   // ui->ContinuecheckBox_2->setVisible(false);
    //ui->ContinuecheckBox_3->setVisible(false);
   // ui->ContinuecheckBox_4->setVisible(false);
   // ui->ContinuecheckBox_5->setVisible(false);
    //ui->ContinuecheckBox_6->setVisible(false);
    //ui->ContinuecheckBox_7->setVisible(false);
}

void AdvanceDialog::USBcheckBox_clicked(int value)
{
    char olddata[2];
    char newdata[2];
    char cmdline[MAX_COMMAND_LEN];
    if(value == 0){
        usbconfig.setValue("USBSTORAGE",0);
    }else{
        usbconfig.setValue("USBSTORAGE",1);
    }
    //get old usbfunctoin value
    if(readfromfile("USBSTORAGE", olddata, USB_CONFIG_PATH)){

        //get currnet checkbox value
        sprintf(newdata,"%d", ui->USBcheckBox->isChecked());
        if(pUtility->savetofile("USBSTORAGE", olddata, newdata, USB_CONFIG_PATH)){

            memset(cmdline,0,MAX_COMMAND_LEN);
            sprintf(cmdline,"sed -i 's/\\ \\/drive:\\*,hotplug//g' %s",RDP_CONF_PATH);
            system(cmdline);

            //Check Drive redirection is Enabled/Disabled
            if(ui->USBcheckBox->isChecked()){

                memset(cmdline,0,MAX_COMMAND_LEN);
                sprintf(cmdline,"echo -n ' /drive:*,hotplug' >> %s",RDP_CONF_PATH);
                system(cmdline);

            }else{
                qDebug("Disable drive redirection");
            }

        }else{
            PRINT_LOG((LOG_ERR,"write usbfunction.conf fail!"));
        }

    }else{
        PRINT_LOG((LOG_ERR,"Read usbfunction.conf fail!"));
    }
}

void AdvanceDialog::urbdrc_checkbox_clicked(int value)
{
    if(value == 0){
        urbdrc.setValue("URBDRC",0);
    }else{
        urbdrc.setValue("URBDRC",2);
    }  
    if(ui->urbdrc_checkbox->isChecked()){
        myProcess_lsusb.start("get_urbdrc.sh");
    }
}

void AdvanceDialog::showFinished_lsusb(int, QProcess::ExitStatus)
{
    char olddata[2];
    char set_to_zero[1];
    char cmdline[MAX_COMMAND_LEN];
    FILE *hopen;
    char ret[255];


    //Get saved list, then remove the command in rdp.conf
     QFileInfo checkFile_urbdrc("/usr/ast/urbdrc_devices.conf");
     if(checkFile_urbdrc.exists()){
         memset(cmdline,0,MAX_COMMAND_LEN);
         memset(ret,0,255);
         sprintf(cmdline,"cat /usr/ast/urbdrc_devices.conf");
         hopen=popen(cmdline,"r");
         while(fgets(ret, sizeof(ret) - 1, hopen) != NULL){}
         sprintf(cmdline,"sed -e \"s/\\ \\%s//g\" -i %s", ret, RDP_CONF_PATH);
         system(cmdline);
     }

    //if any urbdrc devices exist, add the vid/pid to rdp.conf
    QFileInfo checkFile_urbdrc2("/usr/ast/urbdrc_devices.conf");
    if(checkFile_urbdrc2.exists()) {
        memset(cmdline,0,MAX_COMMAND_LEN);
        memset(ret,0,255);
        sprintf(cmdline,"cat /usr/ast/urbdrc_devices.conf");
        hopen=popen(cmdline,"r");
        while(fgets(ret, sizeof(ret) - 1, hopen) != NULL){}
        sprintf(cmdline,"echo -n ' %s' >> %s", ret, RDP_CONF_PATH);
        system(cmdline);
        ui->otherUsbInfo->setText(tr("Found urbdrc devices!"));
    }else{
        //disable the checkbox
        ui->urbdrc_checkbox->setChecked(0);
        //set the urbdrc_function=0
        memset(olddata,0,2);
        readfromfile("URBDRC", olddata, URBDRC_CONFIG_PATH);
        pUtility->savetofile("URBDRC", olddata, set_to_zero, URBDRC_CONFIG_PATH);
        ui->otherUsbInfo->setText(tr("No urbdrc devices exist!"));
    }
    QTimer::singleShot(3000, this, SLOT(hideMsg()));
}

void AdvanceDialog::showFinished_network_update(int, QProcess::ExitStatus)
{
    system("reboot");
}

void AdvanceDialog::showFinishedMac(int, QProcess::ExitStatus)
{
    if(uiconf.value("USE_DHCP").toString() == "0"){
        myProcess.start("modify_ip.sh Static "+uiconf.value("IP_ADDR").toString() + " " + uiconf.value("GATEWAY").toString());
        //qDebug()<<"modify_ip.sh Static "+uiconf.value("IP_ADDR").toString() + " " + uiconf.value("GATEWAY").toString();
    }else if(uiconf.value("USE_DHCP").toString() == "1"){
        myProcess.start("modify_ip.sh DHCP");
        //qDebug()<<"dhcp";
    }
}


void AdvanceDialog::on_systemPassword_clicked()
{
    ui->stackedPassword->setCurrentIndex(0);
}

void AdvanceDialog::on_userPassword_clicked()
{
    ui->stackedPassword->setCurrentIndex(1);
}

void AdvanceDialog::on_bootPassword_clicked()
{
    ui->stackedPassword->setCurrentIndex(2);
}
void AdvanceDialog::initPassword()
{
    /*去掉LineEdit右键菜单*/
    ui->boot_password->setContextMenuPolicy(Qt::NoContextMenu);
    ui->boot_password_confirm->setContextMenuPolicy(Qt::NoContextMenu);
    ui->boot_password_old->setContextMenuPolicy(Qt::NoContextMenu);
    ui->system_password->setContextMenuPolicy(Qt::NoContextMenu);
    ui->system_password_confirm->setContextMenuPolicy(Qt::NoContextMenu);
    ui->system_password_old->setContextMenuPolicy(Qt::NoContextMenu);
    ui->user_password->setContextMenuPolicy(Qt::NoContextMenu);
    ui->user_password_confirm->setContextMenuPolicy(Qt::NoContextMenu);
    ui->user_password_old->setContextMenuPolicy(Qt::NoContextMenu);

    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString user_passwd = settings.value("user_password").toString();
    QString system_passwd = settings.value("system_password").toString();
    QString boot_passwd = settings.value("boot_password").toString();
    ui->user_password_old->clear();//清空输入框中的内容
    ui->user_password->clear();
    ui->user_password_confirm->clear();

    ui->system_password_old->clear();
    ui->system_password->clear();
    ui->system_password_confirm->clear();

    ui->boot_password_old->clear();
    ui->boot_password->clear();
    ui->boot_password_confirm->clear();;

    /*密码输入框为空的时候，显示提示输入内容*/
    ui->system_password->setPlaceholderText(tr("system_password"));
    ui->system_password_confirm->setPlaceholderText(tr("system_password_confirm"));
    ui->user_password->setPlaceholderText(tr("user_password"));
    ui->user_password_confirm->setPlaceholderText(tr("user_password_confirm"));
    ui->boot_password->setPlaceholderText(tr("boot_password"));
    ui->boot_password_confirm->setPlaceholderText(tr("boot_password_confirm"));

    if(!user_passwd.isEmpty()) //若不存在密码信息文件，则old密码框不可用
    {
        ui->user_password_old->setPlaceholderText(tr("user_password_old"));
        ui->user_password_old->setEnabled(true);
    }else{
        ui->user_password_old->setPlaceholderText(tr("Create new password"));
        ui->user_password_old->setEnabled(false);
    }
    if(!system_passwd.isEmpty())
    {
        ui->system_password_old->setPlaceholderText(tr("system_password_old"));
        ui->system_password_old->setEnabled(true);
    }else{
        ui->system_password_old->setPlaceholderText(tr("Create new password"));
        ui->system_password_old->setEnabled(false);
    }
    if(!boot_passwd.isEmpty())
    {
        ui->boot_password_old->setPlaceholderText(tr("boot_password_old"));
        ui->boot_password_old->setEnabled(true);
    }else{
        ui->boot_password_old->setPlaceholderText(tr("Create new password"));
        ui->boot_password_old->setEnabled(false);
    }
}

void AdvanceDialog::on_systemPasswordSave_clicked()
{
    advmsg->setWindowTitle(tr("Information"));
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString system_passwd = settings.value("system_password").toString();
    if(ui->system_password->text().isEmpty()&&ui->system_password_confirm->text().isEmpty()&&ui->system_password_old->text().isEmpty()){
        advmsg->setText(tr("Please enter the password"));
        advmsg->exec();
        return;
    }
    if(!system_passwd.isEmpty()){

        if(system_passwd == ui->system_password_old->text()){
            if(ui->system_password->text() == ui->system_password_confirm->text() && !ui->system_password->text().isEmpty() && !ui->system_password_confirm->text().isEmpty()){
                settings.setValue("system_password",ui->system_password->text());
                 advmsg->setText(tr("Set Password Successful"));
            }else if(ui->system_password->text().isEmpty()&&ui->system_password_confirm->text().isEmpty()){
                settings.setValue("system_password",ui->system_password->text());
                 advmsg->setText(tr("Clear Password Successful"));
            }else{
                 advmsg->setText(tr("Two password are not same!!!"));
            }
        }else{
            advmsg->setText(tr("Password Error,Modify failed"));
        }
    }else{
        if(ui->system_password->text() == ui->system_password_confirm->text()){
            settings.setValue("system_password",ui->system_password->text());
             advmsg->setText(tr("Set Password Successful"));
        }else{
             advmsg->setText(tr("Two password are not same!!!"));
        }
    }
    advmsg->exec();
    initPassword();
}

void AdvanceDialog::on_userPasswordSave_clicked()
{
    advmsg->setWindowTitle(tr("Information"));
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString user_passwd = settings.value("user_password").toString();
    if(ui->user_password->text().isEmpty()&&ui->user_password_confirm->text().isEmpty()&&ui->user_password_old->text().isEmpty()){
        advmsg->setText(tr("Please enter the password"));
        advmsg->exec();
        return;
    }
    if(!user_passwd.isEmpty()){
        if(user_passwd == ui->user_password_old->text()){
            if(ui->user_password->text() == ui->user_password_confirm->text() && !ui->user_password->text().isEmpty() && !ui->user_password_confirm->text().isEmpty()){
                settings.setValue("user_password",ui->user_password->text());
                advmsg->setText(tr("Set Password Successful"));
            }else if(ui->user_password->text().isEmpty()&&ui->user_password_confirm->text().isEmpty()){
                settings.setValue("user_password",ui->user_password->text());
                advmsg->setText(tr("Clear Password Successful"));
            }else{
                advmsg->setText(tr("Two password are not same!!!"));
            }
        }else{
                advmsg->setText(tr("Password Error,Modify failed"));
        }
    }else{
        if(ui->user_password->text() == ui->user_password_confirm->text()){
            settings.setValue("user_password",ui->user_password->text());
            advmsg->setText(tr("Set Password Successful"));
        }else{
            advmsg->setText(tr("Two password are not same!!!"));
        }
    }
    advmsg->exec();
    initPassword();
}

void AdvanceDialog::on_bootPasswordSave_clicked()
{
    advmsg->setWindowTitle(tr("Information"));
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString boot_passwd = settings.value("boot_password").toString();
    if(ui->boot_password->text().isEmpty()&&ui->boot_password_confirm->text().isEmpty()&&ui->boot_password_old->text().isEmpty()){
        advmsg->setText(tr("Please enter the password"));
        advmsg->exec();
        return;
    }
    if(!boot_passwd.isEmpty()){
        if(boot_passwd == ui->boot_password_old->text()){
            if(ui->boot_password->text() == ui->boot_password_confirm->text() && !ui->boot_password->text().isEmpty() && !ui->boot_password_confirm->text().isEmpty()){
                settings.setValue("boot_password",ui->boot_password->text());
                settings.setValue("AUTO_PASSWORD","1");
                advmsg->setText(tr("Set Password Successful"));
            }else if(ui->boot_password->text().isEmpty()&&ui->boot_password_confirm->text().isEmpty()){
                settings.setValue("AUTO_PASSWORD","0");
                settings.setValue("boot_password",ui->boot_password->text());
                advmsg->setText(tr("Clear Password Successful"));
            }else{
                advmsg->setText(tr("Two password are not same!!!"));
            }
        }else{
                advmsg->setText(tr("Password Error,Modify failed"));
        }
    }else{
        if(ui->boot_password->text() == ui->boot_password_confirm->text()){
            settings.setValue("boot_password",ui->boot_password->text());
            settings.setValue("AUTO_PASSWORD","1");
            advmsg->setText(tr("Set Password Successful"));
        }else{
            advmsg->setText(tr("Two password are not same!!!"));
        }
    }
    advmsg->exec();
    initPassword();
}

void AdvanceDialog::hideMsg()
{
    ui->otherUsbInfo->clear();
}


void AdvanceDialog::translateLanguage()
{
    ui->retranslateUi(this);
    initPassword();
}

void AdvanceDialog::onRadioClickAutologin() {
    ui->ContinuecheckBox_1->setCheckable(true);
    ui->ContinuecheckBox_2->setCheckable(true);
    ui->ContinuecheckBox_3->setCheckable(true);
    ui->ContinuecheckBox_4->setCheckable(true);
    ui->ContinuecheckBox_5->setCheckable(true);
    ui->ContinuecheckBox_6->setCheckable(true);
    ui->ContinuecheckBox_7->setCheckable(true);

    ui->ContinuecheckBox_1->setChecked(false);
    ui->ContinuecheckBox_2->setChecked(false);
    ui->ContinuecheckBox_3->setChecked(false);
    ui->ContinuecheckBox_4->setChecked(false);
    ui->ContinuecheckBox_5->setChecked(false);
    ui->ContinuecheckBox_6->setChecked(false);
    ui->ContinuecheckBox_7->setChecked(false);

    ui->ContinuecheckBox_1->setCheckable(false);
    ui->ContinuecheckBox_2->setCheckable(false);
    ui->ContinuecheckBox_3->setCheckable(false);
    ui->ContinuecheckBox_4->setCheckable(false);
    ui->ContinuecheckBox_5->setCheckable(false);
    ui->ContinuecheckBox_6->setCheckable(false);
    ui->ContinuecheckBox_7->setCheckable(false);

    if(pUtility->file_exist(TMP_HOST_DATA))
    {
        system("rm -rf /usr/ast/tmphost");
        system("sync");

    }

    if(btnGroupAutologin->checkedId()==7)
        update_AutoLogin(false);
    else
        update_AutoLogin(true);

    switch(btnGroupAutologin->checkedId())
    {
       case 0:
           //qDebug() << QString::fromLocal8Bit("1");
           userinfo.setValue("AutoLoginnum","1");
           ui->ContinuecheckBox_1->setCheckable(true);

           pUtility->savetofile("username",NULL,userinfo.value("Username1").toString().toLatin1().data(),TMP_HOST_DATA);
           pUtility->savetofile("host",NULL,userinfo.value("ConnectName1").toString().toLatin1().data(),TMP_HOST_DATA);
           emit WritetoRdpconf(userinfo.value("IPAddress1").toString(), userinfo.value("Username1").toString(), userinfo.value("Password1").toString());
        break;
       case 1:
           //qDebug() << QString::fromLocal8Bit("2");
           userinfo.setValue("AutoLoginnum","2");
           ui->ContinuecheckBox_2->setCheckable(true);
           pUtility->savetofile("username",NULL,userinfo.value("Username2").toString().toLatin1().data(),TMP_HOST_DATA);
           pUtility->savetofile("host",NULL,userinfo.value("ConnectName2").toString().toLatin1().data(),TMP_HOST_DATA);
           emit WritetoRdpconf(userinfo.value("IPAddress2").toString(), userinfo.value("Username2").toString(), userinfo.value("Password2").toString());
           break;
       case 2:
           //qDebug() << QString::fromLocal8Bit("3");
           userinfo.setValue("AutoLoginnum","3");
           ui->ContinuecheckBox_3->setCheckable(true);
           pUtility->savetofile("username",NULL,userinfo.value("Username3").toString().toLatin1().data(),TMP_HOST_DATA);
           pUtility->savetofile("host",NULL,userinfo.value("ConnectName3").toString().toLatin1().data(),TMP_HOST_DATA);
           emit WritetoRdpconf(userinfo.value("IPAddress3").toString(), userinfo.value("Username3").toString(), userinfo.value("Password3").toString());
           break;
        case 3:
            //qDebug() << QString::fromLocal8Bit("4");
            userinfo.setValue("AutoLoginnum","4");
            ui->ContinuecheckBox_4->setCheckable(true);
            pUtility->savetofile("username",NULL,userinfo.value("Username4").toString().toLatin1().data(),TMP_HOST_DATA);
            pUtility->savetofile("host",NULL,userinfo.value("ConnectName4").toString().toLatin1().data(),TMP_HOST_DATA);
            emit WritetoRdpconf(userinfo.value("IPAddress4").toString(), userinfo.value("Username4").toString(), userinfo.value("Password4").toString());
            break;
        case 4:
           // qDebug() << QString::fromLocal8Bit("5");
            userinfo.setValue("AutoLoginnum","5");
            ui->ContinuecheckBox_5->setCheckable(true);
            pUtility->savetofile("username",NULL,userinfo.value("Username5").toString().toLatin1().data(),TMP_HOST_DATA);
            pUtility->savetofile("host",NULL,userinfo.value("ConnectName5").toString().toLatin1().data(),TMP_HOST_DATA);
            emit WritetoRdpconf(userinfo.value("IPAddress5").toString(), userinfo.value("Username5").toString(), userinfo.value("Password5").toString());
            break;
        case 5:
            //qDebug() << QString::fromLocal8Bit("6");
            userinfo.setValue("AutoLoginnum","6");
            ui->ContinuecheckBox_6->setCheckable(true);
            pUtility->savetofile("username",NULL,userinfo.value("Username6").toString().toLatin1().data(),TMP_HOST_DATA);
            pUtility->savetofile("host",NULL,userinfo.value("ConnectName6").toString().toLatin1().data(),TMP_HOST_DATA);
            emit WritetoRdpconf(userinfo.value("IPAddress6").toString(), userinfo.value("Username6").toString(), userinfo.value("Password6").toString());
            break;
        case 6:
           // qDebug() << QString::fromLocal8Bit("7");
            userinfo.setValue("AutoLoginnum","7");
            ui->ContinuecheckBox_7->setCheckable(true);
            pUtility->savetofile("username",NULL,userinfo.value("Username0").toString().toLatin1().data(),TMP_HOST_DATA);
            pUtility->savetofile("host",NULL,userinfo.value("ConnectName0").toString().toLatin1().data(),TMP_HOST_DATA);
            emit WritetoRdpconf(userinfo.value("IPAddress0").toString(), userinfo.value("Username0").toString(), userinfo.value("Password0").toString());
            break;
        case 7:
           // qDebug() << QString::fromLocal8Bit("8");
            userinfo.setValue("ContinueLogin","0");
            userinfo.setValue("AutoLoginnum","8");
            break;
    }
}


void AdvanceDialog::on_ContinuecheckBox_1_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_2_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_3_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_4_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_5_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_6_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::on_ContinuecheckBox_7_clicked(bool checked)
{
    if(checked)
       userinfo.setValue("ContinueLogin","1");
    else
       userinfo.setValue("ContinueLogin","0");
}

void AdvanceDialog::update_AutoLogin(bool xautologin)
{

    char olddata[2];
    char newdata[2];

    int old_autologin = 0;
    int m_autologin=1;

    if(xautologin==false)
    {
        old_autologin = 1;
        m_autologin=0;
    }
    memset(olddata, 0, 2);
    memset(newdata, 0, 2);

    if(pUtility->readfromfile("AUTO_LOGIN", olddata, SETTINGS_SETTINGS_CONFIG_PATH))
    {
        old_autologin = atoi(olddata);
        if(old_autologin != m_autologin)
        {
            sprintf(newdata,"%d",m_autologin);
            if(pUtility->savetofile("AUTO_LOGIN", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
            {
                qDebug("AUTO_LOGIN saved ok\n");
            }
        }
    }
    else
    {
        sprintf(newdata,"%d",m_autologin);
        if(pUtility->savetofile("AUTO_LOGIN", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
        {
            qDebug("AUTO_LOGIN saved ok\n");
        }
    }

}

void AdvanceDialog::on_delete_07_clicked()
{
    userinfo.setValue("ConnectName0", "");
    userinfo.setValue("IPAddress0", "");
    userinfo.setValue("Username0", "");
    userinfo.setValue("Password0", "");
    FirstDlg_Init();
}


void AdvanceDialog::on_RemembercheckBox_clicked(bool checked)
{
    char olddata[2];
    char newdata[2];
    memset(olddata,0,2);
    memset(newdata,0,2);
    if(checked)
       sprintf(newdata,"1");
    else
       sprintf(newdata,"0");

    pUtility->savetofile("REMEMBER_ME", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH);
}
