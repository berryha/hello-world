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
#include "tooldialog.h"
#include "ui_tooldialog.h"

ToolDialog::ToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolDialog)
{
    ui->setupUi(this);
    cmdflag=0;

    pUtility = new Utility;
    toolmsg = new QMessageBox;
    toolmsg->setParent(this);
    toolmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(MAX_CMD_WAIT_TIME);
    going=0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(timer_timeout()));
    thread_timer= new QTimer(this);
    emit accepted();
    //init_str();
   // initdata();
}
void ToolDialog::init_str()
{ 	
}
void ToolDialog::initCMD()
{
    workerthread = new QThread;
    worker = new CMDThread;
    connect(worker, SIGNAL(changestat(QString)),ui->pingresulttextEdit,SLOT(append(QString)));
    connect(workerthread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), workerthread, SLOT(quit()));
    connect(workerthread, SIGNAL(finished()),SLOT(onCMDdone()));
    worker->moveToThread(workerthread);
}    
void ToolDialog::initdata()
{
    FILE * macfd;
    char macbuf[32];
    memset(macbuf,0,32);
    macfd=fopen(WOL_MAC_PATH,"r");
    if(macfd!=NULL)
    {
       if(fgets(macbuf,32,macfd)!=NULL)
       {   
       	   if(strlen(macbuf)>0)
       	   {       	   	   
       	   	   ui->wakeupMAC->setText(QString(macbuf));
       	   }
       }
    }
}
void ToolDialog::initial_log()
{
    //qDebug("initial_log");
    //ui->advanceTab->setEnabled(false);
    //qApp->setOverrideCursor(Qt::WaitCursor);
    //ui->logtextBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    //ui->logtextBrowser->clear();
    logFilter();
    //logworkerthread = new QThread;
    //logworker = new logthread;
    //logworker->moveToThread(logworkerthread);
    //connect(logworker, SIGNAL(DataChanged(QString)),ui->logtextBrowser,SLOT(append(QString)));
    //connect(logworkerthread, SIGNAL(started()), logworker, SLOT(run()));
    //connect(logworker, SIGNAL(finished()), logworkerthread, SLOT(quit()));
    //connect(logworkerthread, SIGNAL(finished()),SLOT(onRestoreCursor()));
    //m_timer->singleShot(1, this,SLOT(onLoadStart()));
}
void ToolDialog::onLoadStart()
{
    logworkerthread->start();
}
void ToolDialog::onRestoreCursor()
{
    if(logworker!=NULL)
    {
        free(logworker);
        logworker=NULL;
    }
    if(logworkerthread!=NULL)
    {
        free(logworkerthread);
        logworkerthread=NULL;
    }
    //ui->tooltabWidget->setEnabled(true);
   // qApp->restoreOverrideCursor();
}
ToolDialog::~ToolDialog()
{
    delete ui;
}
void ToolDialog::thread_start()
{
    workerthread->start();
}
void ToolDialog::timer_timeout()
{
    going++;
    if(going==MAX_CMD_WAIT_TIME)
    {
        going=0;
    }
    ui->progressBar->setValue(going);
}

void ToolDialog::onCMDdone()
{
	if(worker!=NULL)
	{
		free(worker);
		worker=NULL;
	}	
	if(workerthread!=NULL)
	{
		free(workerthread);
		workerthread=NULL;
	}		
    m_timer->stop();
    ui->progressBar->setValue(MAX_CMD_WAIT_TIME);
    //enable all component
    //ui->tooltabWidget->setEnabled(true);
    //ui->toolexitButton->setEnabled(true);
    qApp->restoreOverrideCursor();
    cmdflag=0;
}

void ToolDialog::on_pingButton_clicked()
{
    char cmdline[MAX_COMMAND_LEN];
    QString ipaddr;
    char ipbuff[20];
    int linktype=1;
    char netif[8];
    memset(netif,0,8);
    sprintf(netif,"eth0");
    if(pUtility->is_IF_up(netif)!=1)
    {
    	linktype=0;
		memset(netif,0,8);
		sprintf(netif,"wlan0");    	
    }     
    toolmsg->setWindowTitle(tr("Warning"));
    ipaddr=ui->targetIP->text();
    if(ipaddr=="")
    {
        toolmsg->setText(tr("Invalid Host Name/IP address! Please try again."));
        if(toolmsg->exec()==QMessageBox::Ok)
        {
            ui->targetIP->setFocus();
        }    	    
    	    return;
    }
    memset(ipbuff,0,20);    
    ui->pingresulttextEdit->clear();
    ui->progressBar->setValue(0);
    going=0;
    qApp->setOverrideCursor(Qt::WaitCursor);
    //ui->tooltabWidget->setEnabled(false);
    //ui->toolexitButton->setEnabled(false);
    //check user input target ip valid
    do
    {   	  
        if(pUtility->check_link(linktype))
        {
        	//check eth0 got valid IPv4
		if(pUtility->get_currentIP(netif,ipbuff))
		{        
			
		    memset(ipbuff,0,20);
		    if(!pUtility->host_to_ip(ipaddr.toLatin1().data(),ipbuff))
		    {
		    	    //try again nslookup to find ipaddress
		    	    memcpy(ipbuff,ipaddr.toLatin1().data(),strlen(ipaddr.toLatin1().data()));
		    }
		}
		else
		{
            //ui->tooltabWidget->setEnabled(true);
            //ui->toolexitButton->setEnabled(true);
            qApp->restoreOverrideCursor();
            //popup message to user, no network to pin
            toolmsg->setText(tr("No network. Please check network."));
		    toolmsg->exec();
		    break;			
		}		    
        }
        else
        {
//			    ui->tooltabWidget->setEnabled(true);
//			    ui->toolexitButton->setEnabled(true);
			    qApp->restoreOverrideCursor();        	
            //popup message to user, no network to ping
            toolmsg->setText(tr("No network. Please check network."));
            toolmsg->exec();
            break;
        }
        //ping to target 2 times      
         memset(cmdline,0,MAX_COMMAND_LEN);
         sprintf(cmdline,"ping -c 2 -w 5 %s 2>&1",ipbuff);
         
        initCMD();         
        worker->setcmdline(cmdline);
        cmdflag=1;
        m_timer->start(500);
        thread_timer->singleShot(0, this,SLOT(thread_start()));
    }while(0);
}

void ToolDialog::on_advanceTab_currentChanged(int index)
{

    if(!account_flag)
    {
        index++;
    }
    switch(index)
    {
    case 0: //ping
        ui->checkstackedWidget->setCurrentIndex(0);break;
    case 1: //log
        ui->checkstackedWidget->setCurrentIndex(1);break;
    default:
            break;
    }
}

void ToolDialog::on_wakeupButton_clicked()
{
    FILE *fconf;
    FILE *savefd;
    char cmdline[MAX_COMMAND_LEN];
    char ipbuff[20];
    QString macaddr;
    int linktype=1;
    char netif[8];
    memset(netif,0,8);
    sprintf(netif,"eth0");
    if(pUtility->is_IF_up(netif)!=1)
    {
    	linktype=0;
		memset(netif,0,8);
		sprintf(netif,"wlan0");    	
    }    
    macaddr=ui->wakeupMAC->text();
    memset(ipbuff,0,20);
    //check user input target ip valid
    do
    {
        toolmsg->setWindowTitle(tr("Warning"));

        if(pUtility->check_link(linktype))
        {
        	//check eth0 got valid IPv4
		if(pUtility->get_currentIP(netif,ipbuff))
		{            	
		    if(!pUtility->isValidMAC(macaddr))
		    {
			    //Invalid IP address, popup message, and set focus

              toolmsg->setText(tr("Invalid MAC address! Please try again."));
		      if(toolmsg->exec()==QMessageBox::Ok)
		      {
			  ui->wakeupMAC->setFocus();
		      }
		      return;
		    }
		}
		else
		{
		    //popup message to user, no network to ping		
            toolmsg->setText(tr("No network. Please check network."));
		    toolmsg->exec();
		    return;			
		}
        }
        else
        {
            //popup message to user, no network to ping		
            toolmsg->setText(tr("No network. Please check network."));
            toolmsg->exec();
            return;
        }
        //arping to target 2 times, and get mac address
         memset(cmdline,0,MAX_COMMAND_LEN);
         sprintf(cmdline,"ether-wake %s",macaddr.toLatin1().data());
         savefd=fopen(WOL_MAC_PATH,"w");
         if(savefd!=NULL)
         {
         	 fputs(macaddr.toLatin1().data(),savefd);
         	 fclose(savefd);
         	 savefd=0;
         }       
         fconf=popen(cmdline,"r");         
         if(fconf!=NULL)
         {
            pclose(fconf);
            fconf=0;
            toolmsg->setWindowTitle(tr("Warning"));
            toolmsg->setText(tr("Save file to ..."));
            toolmsg->exec();
         }
    }while(0);

}

void ToolDialog::keyPressEvent(QKeyEvent * event)
{
    if(cmdflag)
    {
        if(event->key() != Qt::Key_Escape)
        {
            QDialog::keyPressEvent(event);
        }
    }
    else
    {    
    	if(event->key() == Qt::Key_Escape)
    	{
    		ToolDialog::accept();
    	}
    	else
    	{
    		QDialog::keyPressEvent(event);
        }
    }
}
void ToolDialog::showEvent(QShowEvent * event)
{
    
    QDialog::showEvent(event);
}  

void ToolDialog::on_refreshButton_clicked()
{
    initial_log();
}
int ToolDialog::checkUSB()
{
    DIR* dir;
    struct dirent *dent;
    int usbcount=0;
    dir= opendir(SAVE_ROOT_PATH);
    if (dir)
    {
        while ((dent = readdir(dir)) != NULL)
        {
            struct stat st;
            //avoid display . and ..
            if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
                continue;
                //only display directory, but file
            if (fstatat(dirfd(dir), dent->d_name, &st, 0) < 0)
            {
                //perror(dent->d_name);
                PRINT_LOG((LOG_ERR,"fstatat error:%s\n",strerror(errno)));
                continue;
            }
                usbcount++;
        }
        closedir(dir);
    }
    else
    {
        PRINT_LOG((LOG_ERR,"opendir error:%s\n",strerror(errno)));
    }
    return usbcount;
}
void ToolDialog::on_logsaveButton_clicked()
{
    char cmdline[MAX_COMMAND_LENGHT];
    if(checkUSB())
    {
        //prepare save file
        savetodialog = new SavefileDialog(this);
        savetodialog->setParent(this);
        savetodialog->setFixedSize(250,128);
        savetodialog->setWindowTitle(tr("Save file to ..."));
        savetodialog->setStyleSheet("background-color:Pale gray;");
        savetodialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
        savetodialog->exec();
    }
    else
    {
        toolmsg->setWindowTitle(tr("Warning"));
        memset(cmdline,0,MAX_COMMAND_LEN);
        toolmsg->setText(tr("Please plugin usb mass storage device!"));
        toolmsg->exec();
    }
}

void ToolDialog::on_checkstackedWidget_currentChanged(int index)
{
    switch(index)
    {
    case 0: //Display
            ui->checkstackedWidget->setCurrentIndex(0);break;
    case 1://User
            ui->checkstackedWidget->setCurrentIndex(1);break;
    default:
            break;
    }
}

void ToolDialog::logFilter()
{
    QString list = "Release Date 2017/04/16\n";
    QString list1 = "kernel:CPU A9,Dual Core 1.5GHz\n";
    QString list2 = "kernel:Supported link modes: 10baseT/Half 10baseT/Full\n";
    QString list3 = "100baseT/Half 100baseT/Full 1000baseT/Full\n";
    QString string;
    QFile file("/tmp/eventlog/logfile");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        string = list + list1 + list2 + list3 + file.readAll();
        string = string.replace( QRegExp("aspeed"),"Unicom");
        string = string.replace( QRegExp("ASPEED"),"Unicom");
        string = string.replace( QRegExp("ast"),"MC");
        string = string.replace( QRegExp("AST3200"),"MC5000");
        string = string.replace( QRegExp("asp"),"MC");
        string = string.replace( QRegExp("ASP"),"MC");
        string = string.replace( QRegExp("RDP 7.1"),"RDP 8.1");
        ui->logtextBrowser->setText(string);
        ui->logtextBrowser->moveCursor(QTextCursor::End);  //将接收文本框的滚动条滑到最下面
    }
}

void ToolDialog::translateLanguage()
{
    ui->retranslateUi(this);
}
