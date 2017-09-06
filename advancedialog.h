/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "William Lin" <william_lin@aspeedtech.com>
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
#ifndef ADVANCEDIALOG_H
#define ADVANCEDIALOG_H
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <QTimer>
#include <QDialog>
#include <QProcess>
#include "logthread.h"
#include "dhcp_thread.h"
#include "waitingdialog.h"
#include "updatedialog.h"
#include "utility.h"
#include "savefiledialog.h"
#include "create_users.h"
#include <QButtonGroup>
#define RDP_CONF_PATH			            "/usr/ast/rdp.conf"
#define DEFAULT_SETTINGS_CONFIG_PATH        "/usr/ast/default_setting.conf"
#define SETTINGS_CONFIG_PATH                "/usr/ast/setting.conf"
#define DEFAULT_ACCOUNT_CONFIG_PATH         "/usr/ast/default_account.conf"
#define ACCOUNT_CONFIG_PATH                 "/usr/ast/userinfo.conf"
#define DEFAULT_NET_CONFIG_PATH             "/usr/ast/default_uiconf.conf"
#define NET_CONFIG_PATH                     "/usr/ast/uiconf.conf"
#define DEFAULT_USB_CONFIG_PATH             "/usr/ast/default_usbfunction.conf"
#define USB_CONFIG_PATH                     "/usr/ast/usbfunction.conf"
#define DEFAULT_URBDRC_CONFIG_PATH          "/usr/ast/default_urbdrc_function.conf"
#define URBDRC_CONFIG_PATH                  "/usr/ast/urbdrc_function.conf"
#define DEFAULT_SMARTCARD_CONFIG_PATH       "/usr/ast/default_smartcard_function.conf"
#define SMARTCARD_CONFIG_PATH               "/usr/ast/smartcard_function.conf"
#define DEFAULT_SYSTEM_CONFIG_PATH          "/usr/ast/default_system.conf"

#define MAX_DEVICE_NAME_LENGTH 128
#define MAX_IMG_NAME_LENGTH 128
#define ADV_TIMEOUT	60000 //60 secs

#define ADVANCE_CLOSE		"Close"
#define ADVANCE_TAB_ACCOUNT	"Account"
#define ADVANCE_TAB_NETWORK	"Network"
#define ADVANCE_TAB_USB		"USB"
#define ADVANCE_TAB_RECOVERY	"Recovery"
#define ADVANCE_TAB_UPDATE	"Update"
#define ADVANCE_TAB_SYSTEM	"System"
#define ADVANCE_TAB_LOG		"Log"
#define ADVANCE_ACCOUNT_CURRUSERNAME "Current Username : "
#define ADVANCE_ACCOUNT_CURRPASSWORD "Current Password : "
#define ADVANCE_ACCOUNT_NEWUSERNAME "New Username : "
#define ADVANCE_ACCOUNT_NEWPASSWORD "New Password : "
#define ADVANCE_ACCOUNT_RETYPEPASSWORD "Re-type Password : "
#define ADVANCE_ACCOUNT_APPLY_BUTTON		"Apply"
#define ADVANCE_ACCOUNT_CURRPASSWORD_EMPTY_MSG  "Current password empty! Please input current password."
//#define ADVANCE_ACCOUNT_INVALID_CURRPASSWORD_MSG  "Invalid current password! Please try again."
//#define ADVANCE_ACCOUNT_NEWUSERNAME_EMPTY_MSG  "New username empty! Please input new username."
//#define ADVANCE_ACCOUNT_NEWPASSWORD_EMPTY_MSG  "New password empty! Please input new password."
//#define ADVANCE_ACCOUNT_RETYPEPASSWORD_EMPTY_MSG  "Re-type password empty! Please input re-type password."
//#define ADVANCE_ACCOUNT_RETYPEPNOTMATCH_MSG  "Re-type Password doesn't match with New Password! Please try again."
//#define ADVANCE_ACCOUNT_SAVE_COMPLETELY_MSG		"Save username and password completely."
#define ADVANCE_NETWORK_MAC_LABEL	"MAC Address:"
#define ADVANCE_NETWORK_DHCPIP_LABEL	"Dynamic IP:"
#define ADVANCE_NETWORK_FIXIP_LABEL	"Fixed  IP:"
#define ADVANCE_NETWORK_RENEW_DHCP	"Renew"
#define ADVANCE_NETWORK_MASK_LABEL	"NetMask:"
#define ADVANCE_NETWORK_GATEWAY_LABEL	"Gateway:"
#define ADVANCE_NETWORK_APPLY_BUTTON	"Apply"
//#define ADVANCE_NETWORK_INVALID_IPADDR	"Invalid IP address! Please try again."
//#define ADVANCE_NETWORK_INVALID_NETMASK	"Invalid Netmask! Please try again."
//#define ADVANCE_NETWORK_INVALID_GATEWAY	"Invalid Gateway! Please try again."
//#define ADVANCE_NETWORK_FIXEDIP_SAVED	"Change to Fixed IP mode completed."
#define ADVANCE_USB_STORAGE_REDIRECTION	"Enable USB Storage redirection"
#define ADVANCE_USB_SMARTCARD_REDIRECTION	"Enable USB Smart card reader redirection(refresh every time)"
#define ADVANCE_USB_OTHER_REDIRECTION	"Enable other USB device redirection(refresh every time)"
#define ADVANCE_USB_ENABLE_REDIRECTION	"USB storage redirection enabled."
#define ADVANCE_USB_DISABLE_REDIRECTION	"USB storage redirection disabled."
#define ADVANCE_USB_APPLY_BUTTON	"Refresh"
#define ADVANCE_USB_URBDRC_FOUND	"Other USB devices detected!"
#define ADVANCE_USB_SMARTC_FOUND	"Smart card reader detected!"
#define ADVANCE_USB_URBDRC_NOTFOUND	"No other USB devices detected!"
#define ADVANCE_USB_SMARTC_NOTFOUND	"No smart card reader detected!"
#define ADVANCE_USBFWUPDATE_OVERNUM "Please insert only 1 USB dongle!"
#define ADVANCE_USB_WAIT_REFRESH	"Please wait for refreshing devices."
#define ADVANCE_USB_DESCRIPTION		"(refresh every time)"
#define ADVANCE_RECOVERY_FUCNTION_LABEL	"Recovery to Factory Default Settings:"
#define ADVANCE_RECOVERY_GENERAL_SETTINGS	"General Settings"
#define ADVANCE_RECOVERY_ACCOUNT		"Account"
#define ADVANCE_RECOVERY_NETWORK		"Network"
#define ADVANCE_RECOVERY_USB			"USB Redirection"
#define ADVANCE_RECOVERY_DEVICENAME		"Device Name"
//#define ADVANCE_RECOVERY_MSG			"Recovery completed."
#define ADVANCE_RECOVERY_APPLY_BUTTON		"Apply"
#define ADVANCE_UPDATE_FUNCTION_LABEL		"Firmware Update:"
#define ADVANCE_UPDATE_FOLDER_LABEL		"Image Folder:"
#define ADVANCE_UPDATE_FILE_LABEL		"Image File(*.bin):"
#define ADVANCE_UPDATE_RELOAD_BUTTON		"Reload USB"
#define ADVANCE_UPDATE_BUTTON			"Start Firmware Update"
//#define ADVANCE_UPDATE_NOTICE_MSG		"WARNING! Going to update firmware. Please do NOT power off or shut down the device before the update is completed."
//#define ADVANCE_UPDATE_UPDATING_TITLE		"Firmware Updating..."
#define ADVANCE_UPDATE_NO_VALID_BINARY		"No valid binary found"
#define ADVANCE_SYSTEM_DEVICENAME_LABEL		"Device Name"
#define ADVANCE_SYSTEM_APPLY_BUTTON		"Apply"
#define ADVANCE_SYSTEM_SAVED_MSG		"New device name saved."

//#define ADVANCE_RECOVERY_REBOOT_MSG		"Restart needed for Network recover to take effect."
//#define ADVANCE_UPDATE_CONFIRM_MSG		"Are you sure to update?"
#define ADVANCE_NETWORK_DNS1_LABEL		"DNS1"
//#define ADVANCE_NETWORK_INVALID_DNS1	"Invalid DNS1! Please try again."
#define ADVANCE_NETWORK_DNS2_LABEL		"DNS2"
#define ADVANCE_NETWORK_INVALID_DNS2	"Invalid DNS2! Please try again."
#define ADVANCE_USB_OTHER_USB_FOUND		0x01
#define ADVANCE_USB_OTHER_USB_NOTFOUND	0x02
#define ADVANCE_USB_SMART_CARD_FOUND	0x04
#define ADVANCE_USB_SMART_CARD_NOTFOUND	0x08
enum {FIXIP_MODE,DHCP_MODE};

namespace Ui {
class AdvanceDialog;
}

class AdvanceDialog : public QDialog
{
    Q_OBJECT

public:
	int account_flag;
    QTimer *adv_timer;
    Utility *pUtility;	
    int checkUSB();	
    //int m_kb_fd;
    //unsigned int m_KBD_status;
    QTimer *m_timer;
    logthread   *logworker;
    QThread* logworkerthread;	
    WaitingDialog *waitwindow;
    UpdateDialog *updatewindow;
    Create_Users *createUsers;
    int check_link();
    int savetofile(const char* keyitem,char* olddata,char* newdata,const char* path);
    int readfromfile(const char* keyitem,char* value,const char* path);
    int net_mode;
    QString fixipstr;
    QString netmaskstr;
    QString gatewaystr;
    QString DNS1str;
    QString DNS2str;
    void initial_FWUpdate();
    void initial_USB();
    void initial_account();
    void initial_network();
    void initialdata();
    void initial_system();
    void initial_log();
    void initial_string();
    void FirstDlg_Init();
    void initPassword();
    QString fileUsername;
    QString filePasswd;
    QMessageBox *advmsg;
    explicit AdvanceDialog(QWidget *parent = 0);
    void on_advanceTab_currentChanged(int index);
    ~AdvanceDialog();
    void translateLanguage();


    QButtonGroup *btnGroupAutologin;
signals:
    void change_dhcpmode(int mode);
    void loaddefault();
    void WritetoRdpconf(QString xipaddr, QString xusername, QString xpassword);
public slots:
    //void onRestoreCursor();
    //void onLoadStart();
    void onRadioClickAutologin();
    void update_AutoLogin(bool xautologin);
private slots:
    void on_delete_07_clicked();
    void on_recoveryButton_clicked();
    void on_B_RefreshNet_clicked();
    void on_NET_Submit_Button_clicked(); 
    void on_R_UseDHCP_clicked(bool checked);
    void on_R_FixIP_clicked(bool checked);
    void onDHCPdone();
    void onautodhcp();
    void onNetInvalid();  
    void on_updatepushButton_clicked();
    void on_ImageFolderBox_currentIndexChanged();
    void on_refresh_USB_Btn_clicked();
    void on_New_clicked();
    void DeleteCliecked();
    void USBcheckBox_clicked(int value);
    void urbdrc_checkbox_clicked(int value);
    void showFinished_lsusb(int, QProcess::ExitStatus);
    void showFinished_network_update(int, QProcess::ExitStatus);
    void showFinishedMac(int, QProcess::ExitStatus);
    void on_systemPassword_clicked();
    void on_userPassword_clicked();
    void on_bootPassword_clicked();
    void on_systemPasswordSave_clicked();
    void on_userPasswordSave_clicked();
    void on_bootPasswordSave_clicked();
    void hideMsg();
    void editClicked();

    void on_ContinuecheckBox_1_clicked(bool checked);

    void on_ContinuecheckBox_2_clicked(bool checked);
    void on_ContinuecheckBox_3_clicked(bool checked);
    void on_ContinuecheckBox_4_clicked(bool checked);
    void on_ContinuecheckBox_5_clicked(bool checked);
    void on_ContinuecheckBox_6_clicked(bool checked);
    void on_ContinuecheckBox_7_clicked(bool checked);

    void on_RemembercheckBox_clicked(bool checked);

private:
    Ui::AdvanceDialog *ui;
    QProcess myProcess_lsusb;
    QProcess* myProcess_network_update;
    QProcess myProcess_mac;
    QProcess myProcess;
    QString Random_Mac();
    QTimer *timer;
protected:
    //bool eventFilter(QObject *obj, QEvent *event);
    virtual void showEvent(QShowEvent * event);     
    virtual void keyPressEvent(QKeyEvent * event);
};

#endif // ADVANCEDIALOG_H
