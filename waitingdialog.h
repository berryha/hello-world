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
#ifndef WAITINGDIALOG_H
#define WAITINGDIALOG_H
#include "dhcp_thread.h"
#include <QDialog>
#include <QTimer>
#include <QKeyEvent>
#define MAX_WAIT_TIME 15
namespace Ui {
class WaitingDialog;
}

class WaitingDialog : public QDialog
{
    Q_OBJECT

public:
	void setcmdline(char *pcmd);
	QString cmdstr;
    int going;
    QTimer *m_timer;
    QTimer *thread_timer;
    void initdata();
    dhcp_thread   *worker;
    QThread* workerthread;
    explicit WaitingDialog(QWidget *parent = 0);
    ~WaitingDialog();

private:
    Ui::WaitingDialog *ui;

private slots:
    void thread_start();
    void timer_timeout();	
    void onDHCPdone();
    void onexit();
protected:
virtual void keyPressEvent(QKeyEvent * event);
signals:
    void dhcprenewdone();
};

#endif // WAITINGDIALOG_H
