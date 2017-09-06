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
#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include "updateflash_thread.h"
#include <QDialog>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include "utility.h"
#define MAX_WAIT_UPDATE_TIME 600
//#define UPDATE_STATUS_LABEL "Progress:"
//#define UPDATE_FILE_LABEL "Image:"
#define UPDATE_REBOOT_NOTICE_MSG "Please reboot your system!"
namespace Ui {
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    Utility *pUtility;
    int going;
    QTimer *m_timer;
    QTimer *thread_timer;
    void initdata();
    void setData(char* imgname);
    updateflash_thread *worker;
    QThread* workerthread;
    QMessageBox *advmsg;
    explicit UpdateDialog(QWidget *parent = 0);
    ~UpdateDialog();

private:
    Ui::UpdateDialog *ui;

private slots:
    void thread_start();
    void update_progressbar();
    void onUpdateFlashdone();
    void onexit();
protected:
    virtual void keyPressEvent(QKeyEvent * event);
signals:
    void updateflashdone();
};

#endif // UPDATEDIALOG_H
