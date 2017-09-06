/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "William Lin" <william_lin@aspeedtech.com>
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
#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(QWidget *parent):QDialog(parent),ui(new Ui::UpdateDialog)
{
    char cmdline[MAX_COMMAND_LEN];	
    pUtility = new Utility;
    ui->setupUi(this);
    ui->updatelabel->setText(tr("Progress:"));
    ui->updatefilelabel->setText(tr("Image:"));
    initdata();
}

void UpdateDialog::setData(char* imgname){
    //Display the update image on the update windows
    ui->updatefilenamelabel->setText(imgname);
}

void UpdateDialog::initdata(){

    advmsg = new QMessageBox;
    advmsg->setParent(this);
    advmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

    workerthread = new QThread;
    worker = new updateflash_thread;

    connect(workerthread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), workerthread, SLOT(quit()));
    connect(workerthread, SIGNAL(finished()),SLOT(onUpdateFlashdone()));
    //connect(workerthread, SIGNAL(updatepnt()),SLOT(onUpdatePercent()));//For thread to update the percent.
    worker->moveToThread(workerthread);

    ui->updateprogressBar->setValue(0);
    ui->updateprogressBar->setMinimum(0);
    ui->updateprogressBar->setMaximum(MAX_WAIT_UPDATE_TIME);


    //Every 1000ms, update the progress bar
    going=0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(update_progressbar()));
    m_timer->start(1000);

    thread_timer= new QTimer(this);
    thread_timer->singleShot(0, this,SLOT(thread_start()));

}

void UpdateDialog::thread_start(){
    workerthread->start();
}

void UpdateDialog::update_progressbar(){
    char percentage[10];

    going++;
    if(going==MAX_WAIT_UPDATE_TIME){
        going=0;
    }
    ui->updateprogressBar->setValue(going);
    if(going!=0)
    {
            sprintf(percentage, "%.1f%%", (double)(going*0.16));
    		ui->UpdateStatusLabel->setText(percentage);    		
    }
}

UpdateDialog::~UpdateDialog(){
    delete ui;
}

void UpdateDialog::onUpdateFlashdone(){
    char cmdline[MAX_COMMAND_LEN];
    m_timer->stop();
    thread_timer->stop();

    ui->updateprogressBar->setValue(MAX_WAIT_UPDATE_TIME);
    ui->UpdateStatusLabel->setText("100.0%");
    thread_timer->singleShot(100, this,SLOT(onexit()));
}

void UpdateDialog::onexit(){

    //system("sleep 3");

    //system("reboot");
    UpdateDialog::close();
}

void UpdateDialog::keyPressEvent(QKeyEvent * event){

    if(event->key() != Qt::Key_Escape){
        QDialog::keyPressEvent(event);
    }
}
