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
#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <ioaccess.h>
#include "ast_share.h"
#include <QTime>
//extern QString g_mode[k_mode_num];
static QSettings settings(setting_path,QSettings::IniFormat);
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
extern unsigned int g_board_info;

void SettingDialog::showEvent(QShowEvent * event)
{
    //init_data();
    QDialog::showEvent(event);
}
SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    pUtility = new Utility;
    settingsmsg = new QMessageBox;
    account_flag=1;
    settingsmsg->setParent(this);
    settingsmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    init_string();
    init_data();
    on_tabWidget_currentChanged(0);
    emit autodetectRES(1);
    emit accepted();

}
void SettingDialog::init_string()
{
    int i;
    int PICinuse=0;
    char cmdline[MAX_COMMAND_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromenv("savingpic",cmdline))
    {
    	PICinuse=atoi(cmdline);
    }
    if(PICinuse)
    {
    	ui->wol_checkBox->setEnabled(false);
    }

    for(i=0;i<MAX_HOTKEY_NUM;i++)
    {
        memset(cmdline,0,MAX_COMMAND_LEN);
        if(i==0)
        {
        	sprintf(cmdline,"Ctrl+Pause");
        }
        else
        {
        	sprintf(cmdline,"Ctrl+Shift+F%d",i);
        }
        ui->hotkey_comboBox->addItem(QString(cmdline));
    }	
    ui->bandwidth_combo->clear();
    ui->bandwidth_combo->addItem(SETTINGS_BANDWIDTH_LOW);
    ui->bandwidth_combo->addItem(SETTINGS_BANDWIDTH_HIGE);
    ui->bandwidth_combo->addItem(SETTINGS_BANDWIDTH_WIN2008R2);

    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromenv("hwver",cmdline))
    {
    	m_hwver = atoi(cmdline);
    }
    
    if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
    //if(1)
    {    			
		 memset(cmdline,0,MAX_COMMAND_LEN);

         ui->multiM_radioButton->setText(tr("Extension mode"));
		
		memset(cmdline,0,MAX_COMMAND_LEN);
        if(pUtility->readfromfile("SETTINGS_AUTO_RESOLUTION",cmdline,UI_STRING_PATH))
        {
            ui->dualM_checkBox->setText(QString(cmdline));
        }
        else
        {
            ui->dualM_checkBox->setText(SETTINGS_AUTO_RESOLUTION);
        }
        ui->dualM_checkBox->hide();

    }
    else
    {
        ui->multiM_radioButton->hide();
        ui->dualM_checkBox->hide();
        ui->multiM_radioButton->setEnabled(false);
        ui->dualM_checkBox->setEnabled(false);
        ui->singleM_radioButton->setChecked(true);
    }
	 //ui->bandwidth_combo->setEnabled(false);
    if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
    //if(1)
    {

        ui->singleM_radioButton->setText(tr("Clone mode"));
    }
    else
    {

        ui->singleM_radioButton->setText(tr("Single Display"));
        ui->singleM_radioButton->setChecked(true);
    }

    if(read_auto_power_on()){
        ui->Auto_power_on_checkBox->setChecked(true);
    }else{
        ui->Auto_power_on_checkBox->setChecked(false);
		}
    if(read_language()){
        ui->language->setCurrentIndex(ENGLISH);
    }else{
        ui->language->setCurrentIndex(CHINESE);
    }
}
int SettingDialog::read_wol()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("WOLENABLE",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}
int SettingDialog::read_wokbdmouse()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("WOKBDMOUSE",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}
int SettingDialog::read_autologin()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("AUTO_LOGIN",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}
int SettingDialog::read_audio()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("AUDIO",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}

int SettingDialog::read_evor()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("EVOR",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}

int SettingDialog::read_rememeber_me()
{
    int ret=0;
//    char cmdline[MAX_COMMAND_LEN];
//    if(pUtility->readfromfile("REMEMBER_ME",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
//    {
//        ret=atoi(cmdline);
//    }

    return ret;	
}
SettingDialog::~SettingDialog()
{
    delete ui;
}

int SettingDialog::read_auto_power_on()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("AUTO_POWER_ON",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}



int SettingDialog::read_language()
{
    int ret=0;
    char cmdline[MAX_COMMAND_LEN];
    if(pUtility->readfromfile("LANGUAGE",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
    {
        ret=atoi(cmdline);
    }
    return ret;
}

void SettingDialog::update_settings()
{
	char cmdline[MAX_COMMAND_LEN];
	int waittime=33;
    FILE*   waitf = 0;
    char    mystr[16] = {0};

    waitf = fopen(WAIT_CONF_PATH, "wt");  //wait file for killer
    if(waitf)
    {
		if(pUtility->readfromfile("WAITSECS",cmdline,SETTINGS_SETTINGS_CONFIG_PATH))
		{
			waittime=atoi(cmdline);
		}    	
        sprintf(mystr,"%d",waittime);//fixed 33 seconds
        fputs(mystr, waitf);
        fclose(waitf);
        waitf=0;
    }
}

void SettingDialog::save_settings()
{  
    SaveDeviceName();
    save_auto_power_on();
    save_autologin();
    save_rememberme();
    save_wokbdmouse();
    save_wol();
}
int SettingDialog::save_autologin()
{
	int ret=0;
    char olddata[2];
    char newdata[2];
    int old_autologin = 0;
    memset(olddata, 0, 2);
    memset(newdata, 0, 2);
    if(ui->autologin_checkBox->isChecked())
    {
        m_autologin = 1;
    }
    else
    {
        m_autologin = 0;
    }
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

	return ret;
}
int SettingDialog::save_language()
{
    settings.setValue("LANGUAGE",ui->language->currentIndex());
}
int SettingDialog::save_audio()
{
    int ret=0;
    char olddata[2];
    char newdata[2];
    int old_autologin = 0;
    memset(olddata, 0, 2);
    memset(newdata, 0, 2);

    if(ui->audio_checkBox->isChecked()){
            m_audio=1;
    }else{
            m_audio=0;
    }

    if(pUtility->readfromfile("AUDIO", olddata, SETTINGS_SETTINGS_CONFIG_PATH)){
        old_autologin = atoi(olddata);
        if(old_autologin != m_audio){
            sprintf(newdata, "%d", m_audio);
            if(pUtility->savetofile("AUDIO", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH)){
                qDebug()<<"AUDIO change to "<<m_evor;
            }
        }
    }
    else
    {
        memset(olddata, 0, 2);
        memset(newdata, 0, 2);
        m_audio=0;
        sprintf(newdata, "%d", m_audio);
        if(pUtility->savetofile("AUDIO", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH)){
            qDebug()<<"AUDIO new to "<<m_evor;
        }
    }


    memset(olddata, 0, 2);
    memset(newdata, 0, 2);

    if(ui->evor_checkBox->isChecked()){
            m_evor=1;
    }else{
            m_evor=0;
    }

    if(pUtility->readfromfile("EVOR", olddata, SETTINGS_SETTINGS_CONFIG_PATH)){
        old_autologin = atoi(olddata);
        //if(old_autologin != m_evor){
            sprintf(newdata, "%d", m_evor);
            if(pUtility->savetofile("EVOR", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH)){
                qDebug()<<"EVOR change to "<<m_evor;
            }
        //}
    }
    else
    {

        memset(olddata, 0, 2);
        memset(newdata, 0, 2);
        m_evor=0;
        sprintf(newdata, "%d", m_evor);
        if(pUtility->savetofile("EVOR", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH)){
            qDebug()<<"EVOR new to "<<m_evor;
        }
    }
    return ret;
}
int SettingDialog::save_rememberme()
{
	int ret = 0;
    char olddata[2];
    char newdata[2];
    int old_remember_me = 0;
    memset(olddata,0,2);
    memset(newdata,0,2);
    if(ui->remember_checkBox->isChecked())
    {
        m_remember_me = 1;	
	    //emit Rememberme(1);        
    }
    else
    {
        //emit Rememberme(0);
        m_remember_me = 0;
    }	
//    if(pUtility->readfromfile("REMEMBER_ME", olddata, SETTINGS_SETTINGS_CONFIG_PATH))
//    {
//    	old_remember_me=atoi(olddata);
//    	if(old_remember_me!=m_remember_me)
//    	{
//            sprintf(newdata,"%d",m_remember_me);
//            if(pUtility->savetofile("REMEMBER_ME", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
//            {
//                qDebug("REMEMBER ME saved ok\n");
//            }
//        }
//    }
//    else
//    {
//        sprintf(newdata,"%d",m_remember_me);
//        if(pUtility->savetofile("REMEMBER_ME", olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
//        {
//            qDebug("REMEMBER ME saved ok\n");
//        }
//    }
	return ret;
}
void SettingDialog::onAutoLoginstate()
{
    //read autologin from file
    m_autologin = read_autologin();
    if(m_autologin)
    {
        ui->autologin_checkBox->setChecked(true);
    }
    else
    {
        ui->autologin_checkBox->setChecked(false);
    }
    //read autologin from file
    m_remember_me=read_rememeber_me();
    if(m_remember_me)
    {
        ui->remember_checkBox->setChecked(true);
    }
    else
    {
        ui->remember_checkBox->setChecked(false);
    }
    //read wake on kbd/mouse from file
    m_wokbdmouse=read_wokbdmouse();
    if(m_wokbdmouse)
    {
        ui->wokbdmouse_checkBox->setChecked(true);
    }
    else
    {
        ui->wokbdmouse_checkBox->setChecked(false);
    }    
}
void SettingDialog::on_apply_button_clicked()
{
    //update_settings();
    save_settings();        
    settingsmsg->setText(tr("User settings saved."));
    settingsmsg->setWindowTitle(tr("Information"));
    settingsmsg->exec();	    
}
void SettingDialog::init_data()
{
    int PICinuse=0;
    char cmdline[MAX_COMMAND_LEN];
    m_bkhotkey=DEFAULT_BREAK_HOTKEY;
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromfile(BREAK_KEY,cmdline,SETTING_PATH))
    {
        m_bkhotkey=atoi(cmdline);
    }
    ui->hotkey_comboBox->setCurrentIndex(m_bkhotkey);

    ui->resolution_comboBox->setMaxCount(MAX_RESOLUTION_MODE);
    ui->resolution_comboBox->setMaxVisibleItems(MAX_RESOLUTION_MODE);
    for(int i = 0; i < MAX_RESOLUTION_MODE; i++)
    {
        ui->resolution_comboBox->addItem(g_mode[i]);
    }    
    //check multi-monitor enable or not
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromfile(MULTI_MONITOR, cmdline, SETTING_PATH))
    {
        m_multi_monitor = atoi(cmdline);
    }
    else
    {
        m_multi_monitor = 0;    
    }
    
    if(m_multi_monitor)//dual monitor
    {
    	ui->multiM_radioButton->setChecked(true);
    	ui->singleM_radioButton->setChecked(false);
        ui->resolution_checkBox->setEnabled(true);//auto resolution check box

        m_resolution=0;
        m_autoresolution=0;
        memset(cmdline, 0, MAX_COMMAND_LEN);
        if(pUtility->readfromfile(AUTO_RESOLUTION,cmdline,SETTING_PATH))
        {
            m_autoresolution=atoi(cmdline);
        }
        if(m_autoresolution == 1)//auto detect
        {
            memset(cmdline, 0, MAX_COMMAND_LEN);

            if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
            {
                emit autodetectRES(1);
            }
            else
            {
                emit autodetectRES(0);
            }
            ui->resolution_checkBox->setChecked(true);
            ui->resolution_comboBox->setEnabled(false);
        }
        else
        {
            ui->resolution_checkBox->setChecked(false);
            ui->resolution_comboBox->setEnabled(true);
            //modify for resolution save issue,
            memset(cmdline,0,MAX_COMMAND_LEN);
            if(pUtility->readfromfile(SCREEN_RESOLUTION, cmdline, SETTING_PATH))
            {
                m_resolution=atoi(cmdline);
            }
            ui->resolution_comboBox->setCurrentIndex(m_resolution);
        }
    }
    else //single monitor
    {
    	ui->multiM_radioButton->setChecked(false);
    	ui->singleM_radioButton->setChecked(true);
    	ui->resolution_checkBox->setEnabled(true);//auto resolution check box   

        m_resolution=0;    
        m_autoresolution=0;
        memset(cmdline, 0, MAX_COMMAND_LEN);
        if(pUtility->readfromfile(AUTO_RESOLUTION,cmdline,SETTING_PATH))
        {
            m_autoresolution=atoi(cmdline);
        }   
        if(m_autoresolution == 1)//auto detect
        {
            memset(cmdline, 0, MAX_COMMAND_LEN);
            
            if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
            {
                emit autodetectRES(1);
            }
            else
            {
                emit autodetectRES(0);
            }
            ui->resolution_checkBox->setChecked(true);
            ui->resolution_comboBox->setEnabled(false);
        }
        else    	    
        {
            ui->resolution_checkBox->setChecked(false);
            ui->resolution_comboBox->setEnabled(true);
            //modify for resolution save issue,
            memset(cmdline,0,MAX_COMMAND_LEN);
            if(pUtility->readfromfile(SCREEN_RESOLUTION, cmdline, SETTING_PATH))
            {		
                m_resolution=atoi(cmdline);
            }        
            ui->resolution_comboBox->setCurrentIndex(m_resolution);  		       
        } 
    }    
    //add for resolution save issue,

    //read bandwidth   
    m_bandwidth=0;
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromfile(BANDWIDTH,cmdline,SETTING_PATH))
    {
        m_bandwidth=atoi(cmdline);
    }
    if(m_bandwidth==0)
        ui->evor_checkBox->setVisible(true);
    else
    {
        ui->evor_checkBox->setVisible(false);
        ui->evor_checkBox->setChecked(false);
    }
    ui->bandwidth_combo->setCurrentIndex(m_bandwidth);  
    //read autologin from file
    m_autologin=read_autologin();
    if(m_autologin)
    {
        ui->autologin_checkBox->setChecked(true);
    }
    else
    {
        ui->autologin_checkBox->setChecked(false);
    }
    //read audio onoff from file
    m_audio=read_audio();
    if(m_audio)
    {
        ui->audio_checkBox->setChecked(true);
    }
    else
    {
        ui->audio_checkBox->setChecked(false);
    }

    m_evor=read_evor();
    if(m_evor)
    {
        ui->evor_checkBox->setChecked(true);
    }
    else
    {
        ui->evor_checkBox->setChecked(false);
    }

    //read autologin from file
    m_remember_me=read_rememeber_me();
    if(m_remember_me)
    {
        ui->remember_checkBox->setChecked(true);
    }
    else
    {
        ui->remember_checkBox->setChecked(false);
    } 
    //read wake on kbd mouse from file
    m_wokbdmouse=read_wokbdmouse();
    if(m_wokbdmouse)
    {
        ui->wokbdmouse_checkBox->setChecked(true);
    }else{
        ui->wokbdmouse_checkBox->setChecked(false);
    } 
    if(pUtility->readfromenv("savingpic",cmdline))
    {
    	PICinuse=atoi(cmdline);
    }
    if(PICinuse)
    {
    	ui->wol_checkBox->setChecked(true);
    }
    else
    {
		//read wake on kbd mouse from file
		m_wol=read_wol();
		if(m_wol)
		{
			ui->wol_checkBox->setChecked(true);
		}else{
			ui->wol_checkBox->setChecked(false);
		}
    }
    auto_power_on = read_auto_power_on();
    if(auto_power_on){
        ui->Auto_power_on_checkBox->setChecked(false);
    }else{
        ui->Auto_power_on_checkBox->setChecked(true);
    }

    save_settings();

}
void SettingDialog::on_resolution_checkBox_stateChanged(int arg1)
{

    char cmdline[MAX_COMMAND_LEN];
    int  resolution = 0;    
    if(arg1)
    {

        ui->resolution_comboBox->setEnabled(false);
        //printf("ui, setting dialog, m_hwver=%x\n", m_hwver);
        
        memset(cmdline,0,MAX_COMMAND_LEN);

        if(g_board_info & (1 << MMAVAIABLE_FLAG_BIT))
        {
            emit autodetectRES(1);
            qDebug()<<"autodetectRES(1)";
        }
        else{
            emit autodetectRES(0);
            qDebug()<<"autodetectRES(0)";
        }
        //change resolution combobox
    }
    else
    {
        ui->resolution_comboBox->setEnabled(true);
        
        memset(cmdline,0,MAX_COMMAND_LEN);
        if(pUtility->readfromfile(SCREEN_RESOLUTION,cmdline,SETTING_PATH))
        {
            resolution = atoi(cmdline);
        }
        ui->resolution_comboBox->setCurrentIndex(resolution);        
    }
}

#if 1
void SettingDialog::onResDetect(int size)
{
    qDebug()<<"size="<<size<<endl;
    ui->resolution_comboBox->setCurrentIndex(size);     
}
#endif 



void SettingDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Escape)
    {
    		SettingDialog::accept();
    		return;
    }
    else
    {
    		QDialog::keyPressEvent(event);
    }
}  


int SettingDialog::save_wokbdmouse()
{
	int ret=0;
    char olddata[2];
    char newdata[2];
    int old_wokbdmouse=0;
    memset(olddata,0,2);
    memset(newdata,0,2);
    if(ui->wokbdmouse_checkBox->isChecked())
    {
    	    m_wokbdmouse=1;
    }
    else
    {
    	    m_wokbdmouse=0;
    }	    
    if(pUtility->readfromfile("WOKBDMOUSE",olddata,SETTINGS_SETTINGS_CONFIG_PATH))
    {
    	old_wokbdmouse=atoi(olddata);
    	if(old_wokbdmouse!=m_wokbdmouse)
    	{
		sprintf(newdata,"%d",m_wokbdmouse);
		if(pUtility->savetofile("WOKBDMOUSE",olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
		{
            qDebug("WOKBDMOUSE saved ok\n");
		}
        }
    }
    else
    {
        sprintf(newdata,"%d",m_wokbdmouse);
        if(pUtility->savetofile("WOKBDMOUSE",olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
        {
            qDebug("WOKBDMOUSE saved ok\n");
        }
    }
	return ret;
}
int SettingDialog::save_bandwidth()
{
	int ret=0;
    char olddata[2];
    char newdata[2];
    int  bandwidth=0;
    bandwidth = ui->bandwidth_combo->currentIndex();    
    memset(olddata,0,2);    
    
    if(pUtility->readfromfile(BANDWIDTH, olddata, SETTINGS_SETTINGS_CONFIG_PATH))
    {
    	m_bandwidth = atoi(olddata);
        
    	if(bandwidth != m_bandwidth)
    	{		
            memset(newdata, 0, 2);
			sprintf(newdata, "%d", bandwidth);
			if(pUtility->savetofile(BANDWIDTH, olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
			{
                qDebug("BANDWIDTH saved ok\n");
			}			
        }
    }
    else
    {
        memset(newdata, 0, 2);
        sprintf(newdata, "%d", bandwidth);
        if(pUtility->savetofile(BANDWIDTH, olddata, newdata, SETTINGS_SETTINGS_CONFIG_PATH))
        {
            qDebug("BANDWIDTH saved ok\n");
        }
    }
	return ret;
}
int SettingDialog::save_wol()
{
	int ret=0;
    char olddata[2];
    char newdata[2];
    char envdata[2];
    int old_wol=0;
    int envpwr;
    memset(olddata,0,2);
    memset(newdata,0,2);
    if(ui->wol_checkBox->isChecked())
    {
    	    m_wol=1;
    }
    else
    {
    	    m_wol=0;
    }	    
    if(pUtility->readfromfile("WOLENABLE",olddata,SETTINGS_SETTINGS_CONFIG_PATH))
    {
    	old_wol=atoi(olddata);
    	if(old_wol!=m_wol)
    	{
				sprintf(newdata,"%d",m_wol);
				if(pUtility->savetofile("WOLENABLE",olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
				{
                    qDebug("WOLENABLE saved ok\n");
				}
				memset(envdata,0,2);
				envpwr=0;
				if(pUtility->readfromenv("pwrsaving",envdata))
				{
						envpwr=atoi(envdata);
						if(envpwr)
						{
								//also set wol to uboot config
								if(pUtility->writetoenv("wol",newdata))
								{
                                    qDebug("env wol saved ok\n");
								}
						}					
				}				
        }
    }
    else
    {
        sprintf(newdata,"%d",m_wol);
        if(pUtility->savetofile("WOLENABLE",olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
        {
            qDebug("WOLENABLE saved ok\n");
        }
        memset(envdata,0,2);
        envpwr=0;
        if(pUtility->readfromenv("pwrsaving",envdata))
        {
                envpwr=atoi(envdata);
                if(envpwr)
                {
                        //also set wol to uboot config
                        if(pUtility->writetoenv("wol",newdata))
                        {
                            qDebug("env wol saved ok\n");
                        }
                }
        }

    }
	return ret;
}


void SettingDialog::on_singleM_radioButton_clicked(bool checked)
{
	if(checked)
    {
    	//ui->multiM_radioButton->setChecked(false);
    	//ui->singleM_radioButton->setChecked(true);
    	ui->resolution_checkBox->setEnabled(true);//auto resolution check box

        if(ui->resolution_checkBox->isChecked())
            ui->resolution_comboBox->setEnabled(false);
        else
            ui->resolution_comboBox->setEnabled(true);
    }
}

void SettingDialog::on_multiM_radioButton_clicked(bool checked)
{
	if(checked)
    {
    	//ui->multiM_radioButton->setChecked(true);
    	//ui->singleM_radioButton->setChecked(false);
        ui->resolution_checkBox->setEnabled(true);//auto resolution check box
        if(ui->resolution_checkBox->isChecked())
            ui->resolution_comboBox->setEnabled(false);
        else
            ui->resolution_comboBox->setEnabled(true);
    }
}
int SettingDialog::save_multi_Monitor()
{
	int ret=0;
    char olddata[2];
    char newdata[2];
    int old_multiMon=0;
    memset(olddata,0,2);
    memset(newdata,0,2);
    if(ui->multiM_radioButton->isChecked())
    {
    	    m_multi_monitor=1;
    }
    else
    {
    	    m_multi_monitor=0;
    }	    
    if(pUtility->readfromfile(MULTI_MONITOR,olddata,SETTINGS_SETTINGS_CONFIG_PATH))
    {
    	old_multiMon=atoi(olddata);
//    	if(old_multiMon!=m_multi_monitor)
//    	{
			sprintf(newdata,"%d",m_multi_monitor);
			if(pUtility->savetofile(MULTI_MONITOR,olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
			{
				//qDebug("m_multi_monitor saved ok\n");
			}
//       }
    }
    else
    {
        memset(olddata,0,2);
        memset(newdata,0,2);
        sprintf(newdata,"%d",m_multi_monitor);
        if(pUtility->savetofile(MULTI_MONITOR,olddata,newdata,SETTINGS_SETTINGS_CONFIG_PATH))
        {
            //qDebug("m_multi_monitor saved ok\n");
        }
    }

	return ret;
}
void SettingDialog::SaveDeviceName()
{
    char cmdline[MAX_COMMAND_LENGHT];
    char oldname[MAX_DEVICE_NAME_LENGTH];
    QString qdevicename;
    //save device name to file
    qdevicename=ui->devicenamelineEdit->text();
    memset(oldname,0,MAX_DEVICE_NAME_LENGTH);
     if(pUtility->savetofile("DEVICE_NAME",oldname,qdevicename.toLatin1().data(),SYSTEM_CONFIG_PATH))
    {
        qdevicename.replace("\\","\\\\");
        qdevicename.replace("\"","\\\"");
        qdevicename.replace("\`","\\\`");
            if(pUtility->savetofile("DEVICE_NAME",oldname,qdevicename.toLatin1().data(),SYSTEM_CONFIG_PATH))
            {
                    //message to show save complete
                memset(cmdline,0,MAX_COMMAND_LEN);
                if(pUtility->readfromfile("MESSAGE_INFO_TITLE",cmdline,UI_STRING_PATH))
                {
                    //advmsg->setWindowTitle(QString(cmdline));
                }
                else
                {
                    //advmsg->setWindowTitle(MESSAGE_INFO_TITLE);
                }
                memset(cmdline,0,MAX_COMMAND_LEN);
                if(pUtility->readfromfile("ADVANCE_SYSTEM_SAVED_MSG",cmdline,UI_STRING_PATH))
                {
                    //advmsg->setText(QString(cmdline));
                }
                else
                {
                    //advmsg->setText(ADVANCE_SYSTEM_SAVED_MSG);
                }
                //advmsg->exec();
            }
    }
}
void SettingDialog::initial_system()
{
    qDebug()<<"";
    char devname[MAX_DEVICE_NAME_LENGTH];
    ui->devicenamelineEdit->setReadOnly(false);
    //max device name length 16 bytes
    ui->devicenamelineEdit->setMaxLength(16);
    //save device name to file
    memset(devname,0,MAX_DEVICE_NAME_LENGTH);
    if(advanceDialog->readfromfile("DEVICE_NAME",devname,SYSTEM_CONFIG_PATH))
    {
        ui->devicenamelineEdit->setText(QString(devname));
    }
}

int SettingDialog::save_auto_power_on()
{
    int ret=0;
    QString List;
    if(ui->Auto_power_on_checkBox->isChecked())
    {
            auto_power_on = 0;
                  qDebug()<<"save_auto_power_on222"<<auto_power_on<<endl;
    }
    else
    {

            auto_power_on = 1;
               qDebug()<<"save_auto_power_on111"<<auto_power_on<<endl;
    }

    settings.setValue("AUTO_POWER_ON",auto_power_on);
    List = "fw_setenv pwrsaving " + QString::number(auto_power_on, 10);
    qDebug()<<List;
    QByteArray ba =List.toLatin1();
    const char *list = ba.data();
    system(list);
    return ret;
}


void SettingDialog::on_tabWidget_currentChanged(int index)
{
    if(!account_flag)
    {
        index++;
    }
    switch(index)
    {
    case 0: //Display
            ui->stackedWidget->setCurrentIndex(0);break;
    case 1://User
            ui->stackedWidget->setCurrentIndex(1);break;
    default:
            break;
    }
}


void SettingDialog::on_apply_display_button_clicked()
{
    int n_bkhotkey;
    int n_resolution;
    int n_autoresolution;
    char cmdline[MAX_COMMAND_LEN];
    char olddata[MAX_DATA_LEN];
    char newdata[MAX_DATA_LEN];
    memset(cmdline,0,MAX_COMMAND_LEN);
    //check hotkey
    n_bkhotkey=ui->hotkey_comboBox->currentIndex();
    if(n_bkhotkey!=m_bkhotkey)
    {
        //save new data to file
        memset(olddata,0,MAX_DATA_LEN);
        memset(newdata,0,MAX_DATA_LEN);
        sprintf(olddata,"%d",m_bkhotkey);
        sprintf(newdata,"%d",n_bkhotkey);
        if(pUtility->savetofile(BREAK_KEY,olddata,newdata,SETTING_PATH))
        {
            m_bkhotkey=n_bkhotkey;
        }
    }
    //auto detect resolution for RDP
    if(ui->resolution_checkBox->isChecked())
    {
      n_autoresolution=1;
    }
    else
    {
      n_autoresolution=0;
    }
    if(n_autoresolution!=m_autoresolution)
    {
        memset(olddata,0,MAX_DATA_LEN);
        memset(newdata,0,MAX_DATA_LEN);
        sprintf(olddata,"%d",m_autoresolution);
        sprintf(newdata,"%d",n_autoresolution);
        //save new data to file
        if(pUtility->savetofile(AUTO_RESOLUTION,olddata,newdata,SETTING_PATH))
        {
            m_autoresolution=n_autoresolution;
        }
    }

    //save resolution
    n_resolution=ui->resolution_comboBox->currentIndex();
    if(n_resolution!=m_resolution)
    {
        memset(olddata,0,MAX_DATA_LEN);
        memset(newdata,0,MAX_DATA_LEN);
        sprintf(olddata,"%d",m_resolution);
        sprintf(newdata,"%d",n_resolution);
        //save new data to file
        if(pUtility->savetofile(SCREEN_RESOLUTION,olddata,newdata,SETTING_PATH))
        {
            //add for also modify rdp.conf for autologin
            memset(cmdline,0,MAX_COMMAND_LEN);
            sprintf(cmdline,"sed -i 's/%s/%s/g' %s",g_mode[m_resolution].toLatin1().data(),g_mode[n_resolution].toLatin1().data(),RDP_CONF_PATH);
            system(cmdline);
            m_resolution=n_resolution;
        }
    }
    save_language();
    save_bandwidth();
    save_multi_Monitor();
    save_audio();

    currentlanguage = ui->language->currentIndex();
    qDebug()<<"SettingDialog emit changeLanguage signal"<<ui->language->currentIndex();
    emit changeLanguage(ui->language->currentIndex());

    settingsmsg->setText(tr("Display settings saved."));
    settingsmsg->setWindowTitle(tr("Information"));
    settingsmsg->exec();
}

void SettingDialog::translateLanguage()
{       
    ui->retranslateUi(this);
    ui->language->setCurrentIndex(currentlanguage);
}


void SettingDialog::on_bandwidth_combo_currentIndexChanged(int index)
{
    if(index==0)
        ui->evor_checkBox->setVisible(true);
    else
    {
        ui->evor_checkBox->setVisible(false);
        ui->evor_checkBox->setChecked(false);
    }
}

void SettingDialog::on_Failed_reconnection_checkBox_stateChanged(int value)
{
    settings.setValue("RECONNECTION",value);
}
