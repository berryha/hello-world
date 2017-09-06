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
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include <QTimer>
#include <QDialog>
#include <sys/ioctl.h>
#include "utility.h"
#include "advancedialog.h"
#define MAX_DATA_LEN		16
#define MAX_HOTKEY_NUM          13
#define MAX_RESOLUTION_MODE     10
#define DEFAULT_TIMEOUT_VALUE   33
#define DEFAULT_BREAK_HOTKEY    0
#define MAX_DEVICE_NAME_LENGTH 128
#define MAX_COMMAND_LENGHT 256
#define CHINESE           0
#define ENGLISH           1
#define BREAK_KEY			"BKHOTKEY"
#define AUTO_RESOLUTION		"AUTORESOLUTION"
#define SCREEN_RESOLUTION	"SIZE"
#define MULTI_MONITOR		"MULTI_MONITOR"
#define BANDWIDTH			"BANDWIDTH"
#define SETTINGS_HOKEY_LABEL		"Remote Connection Breakdown Hotkey: ctrl+alt+"
#define SETTINGS_RESOLUTION_LABEL	"RDP display settings"//"Monitor Resolution:"
#define SETTINGS_AUTO_RESOLUTION	"Auto"
#define SETTINGS_BANDWIDTH_LABEL	"Bandwidth:"
#define SETTINGS_BANDWIDTH_LOW		"RDP8.1"
#define SETTINGS_BANDWIDTH_HIGE		"RDP7.1"
#define SETTINGS_BANDWIDTH_WIN2008R2    "Windows2008R2(RDP7.1 Only)"
#define SETTINGS_SECCONDS		"secs"
//#define SETTINGS_APPLY_MSG		"General settings saved."
#define SETTINGS_APPLY_BUTTON		"Apply"
#define SETTINGS_CLOSE_BUTTON		"Close"
#define SETTINGS_AUTOLOGIN		"Auto-Login"
#define SETTINGS_REMEMBER_ME		"Remember me"
#define SETTINGS_CONNECTION_SETTINGS	"Connection settings"
#define SETTINGS_AUDIO_SETTINGS         "RDP audio settings"
#define SETTINGS_POWER_SETTINGS			"Power settings"
#define SETTINGS_WOKBDMOUSE				"Wake on Keyboard/Mouse"
#define SETTINGS_AUDIO                  "Enable Audio Input and Output"
#define SETTINGS_WOL					"Wake on LAN"
#define SETTINGS_DESKTOP_GROUP			"Desktop settings"
#define SETTINGS_BACKGROUND				"Wallpaper:"
#define SETTINGS_BG_DESCRIPTION			"PNG file only, and size < 1.5MB"
#define SETTINGS_BACKGROUND_BTN			"Modify"
#define SETTINGS_SINGLE_MONITOR_DD      "Single Display"
#define SETTINGS_SINGLE_MONITOR			"Clone mode"
#define SETTINGS_DUAL_MONITOR			"Extension mode"
#define SETTINGS_NET_CONFIG_PATH	"/usr/ast/uiconf.conf"
#define SETTINGS_SETTINGS_CONFIG_PATH	"/usr/ast/setting.conf"
namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public: 
	AdvanceDialog *advanceDialog;
	int account_flag;
	int auto_power_on;
    int m_wol;
    int save_multi_Monitor();
    int read_wol();
    int save_wol();
    int read_wokbdmouse();
    int save_wokbdmouse();
    int read_autologin();
    int read_audio();
    int read_evor();
    int m_autologin;
    int m_audio;
    int m_evor;
    int save_autologin();
    int save_audio();
    int read_rememeber_me();
    int m_remember_me;	
    int save_rememberme();
    int save_bandwidth();
	int m_wokbdmouse;
    void init_string();
    QMessageBox *settingsmsg;
    int m_bkhotkey;
    int m_multi_monitor;
    int m_resolution;
    int m_autoresolution;
    int m_bandwidth;
    Utility *pUtility;
    void save_settings();
    void update_settings();
    void init_data();
 	void SaveDeviceName();
    void initial_system();

    void on_tabWidget_currentChanged(int index);
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();
    void translateLanguage();
    int currentlanguage;

public slots:
	void onResDetect(int size);   
    void onAutoLoginstate();

private slots:
    void on_apply_button_clicked();

    void on_resolution_checkBox_stateChanged(int arg1);
	
	void on_apply_display_button_clicked();
	
    void on_singleM_radioButton_clicked(bool checked);

    void on_multiM_radioButton_clicked(bool checked);
    void on_bandwidth_combo_currentIndexChanged(int index);

    void on_Failed_reconnection_checkBox_stateChanged(int value);

private:
    Ui::SettingDialog *ui;
    int m_hwver;
signals:
    void accepted();
    void dlgclosed(int ctrl);
    void autodetectRES(int multiM);
    //void Rememberme(int remember);
    void changeLanguage(int language);
protected: 
    int save_auto_power_on();
    int read_auto_power_on();
    int save_language();
    int read_language();

    virtual void showEvent(QShowEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);        
};

#endif // SETTINGDIALOG_H
