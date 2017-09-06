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
#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include "utility.h"
#include "cmdthread.h"
#include <QDialog>
#include <QMessageBox>
#include "savefiledialog.h"
#define MAX_CMD_WAIT_TIME 12
#define WOL_MAC_PATH	"/usr/ast/wol.conf"
#define TOOL_TAB_PING	"Ping Tool"
#define TOOL_TAB_WOL	"Log"
#define TOOL_PING_TARGET_LABEL	"Target Host Name/IP Address:"
#define TOOL_PING_STATUS_LABEL	"Status:"
#define TOOL_PING_RESULT_LABEL	"Result:"
#define TOOL_PING_BUTTON	"Start"
#define TOOL_WOL_TARGET_LABEL	"Host PC MAC Address:"
#define TOOL_WOL_BUTTON		"Wake Up"
#define TOOL_CLOSE_BUTTON	"Close"
//#define TOOL_PING_INVALID_TARGET	"Invalid Host Name/IP address! Please try again."
//#define TOOL_PING_NO_NETWORK	"No network. Please check network."
//#define TOOL_WOL_INVALID_MAC	"Invalid MAC address! Please try again."
#define TOOL_WOL_SEND_MSG	"Wake-up command is sent."
#define ADVANCE_LOG_REFRESH_BUTTON		"Refresh"
#define ADVANCE_LOG_SAVETOUSB			"Save to USB"
//#define ADVANCE_LOG_SAVEDIAGLOG_TITLE		"Save file to ..."
//#define ADVANCE_LOG_NOUSB_NOTICE		"Please plugin usb mass storage device!"
namespace Ui {
class ToolDialog;
}

class ToolDialog : public QDialog
{
    Q_OBJECT

public:
    int account_flag;
    void init_str();
    void initCMD();	
    void initdata();
    void initial_log();
    int checkUSB();
    int going;
    CMDThread   *worker;
    QThread* workerthread;
    SavefileDialog *savetodialog;
    logthread   *logworker;
    QThread* logworkerthread;
    int cmdflag;
    QTimer *m_timer;
    QTimer *thread_timer;
    QMessageBox *toolmsg;
    Utility *pUtility;
    explicit ToolDialog(QWidget *parent = 0);
    ~ToolDialog();
     void on_advanceTab_currentChanged(int index);
     void translateLanguage();
private slots:
    void on_pingButton_clicked();

    void on_wakeupButton_clicked();

    void onCMDdone();
    void timer_timeout();
    void thread_start();

    void on_refreshButton_clicked();
    void onRestoreCursor();
    void onLoadStart();

    void on_logsaveButton_clicked();
public slots:
    void on_checkstackedWidget_currentChanged(int index);

private:
    Ui::ToolDialog *ui;
    void logFilter();
protected:
virtual void keyPressEvent(QKeyEvent * event);
virtual void showEvent(QShowEvent * event);
signals:
    void accepted();
};

#endif // TOOLDIALOG_H
