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
#include "waitingdialog.h"
#include "ui_waitingdialog.h"

WaitingDialog::WaitingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaitingDialog)
{
    ui->setupUi(this);

    initdata();
}
void WaitingDialog::setcmdline(char *pcmd)
{
	cmdstr=QString(pcmd);
}
void WaitingDialog::initdata()
{
    workerthread = new QThread;
    worker = new dhcp_thread;
    connect(workerthread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), workerthread, SLOT(quit()));
    connect(workerthread, SIGNAL(finished()),SLOT(onDHCPdone()));
    worker->moveToThread(workerthread);
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(MAX_WAIT_TIME);
    going=0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(timer_timeout()));
    m_timer->start(500);
    thread_timer= new QTimer(this);
    thread_timer->singleShot(0, this,SLOT(thread_start()));
}
void WaitingDialog::thread_start()
{
    worker->setcmdline(cmdstr.toLatin1().data());
    workerthread->start();	
}
void WaitingDialog::timer_timeout()
{
    going++;
    if(going==MAX_WAIT_TIME)
    {
        going=0;
    }
    ui->progressBar->setValue(going);
}

WaitingDialog::~WaitingDialog()
{
    delete ui;
}
void WaitingDialog::onDHCPdone()
{   	
    m_timer->stop();
    ui->progressBar->setValue(MAX_WAIT_TIME);
    emit dhcprenewdone(); 
    thread_timer->singleShot(100, this,SLOT(onexit()));
    
}
void WaitingDialog::onexit()
{
    WaitingDialog::close();
}
void WaitingDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(event);
    }
}
