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
#ifndef SAVEFILEDIALOG_H
#define SAVEFILEDIALOG_H
#include <dirent.h>
#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <stdio.h> 
#include "logthread.h"
#include "utility.h"
#define SAVE_ROOT_PATH "/media/"
#define MAX_SAVE_PATH_LENGTH 256
#define MAX_PROGRESS_STEP 60
#define SAVEDLG_SAVETO_LABEL	"Save to:"
#define SAVEDLG_FILENAME_LABEL	"Filename:"
#define SAVEDLG_SAVE_BUTTON	"Save"
#define SAVEDLG_CLOSE_BUTTON	"Close"
//#define SAVEDLG_FILENAME_EMPTY_MSG "Please input filename!"
//#define SAVEDLG_FILENAME_DUPLICATE_MSG "A duplicate file name has existed! Please rename your file."
//#define SAVEDLG_NOUSB_NOTICE		"Please plugin usb mass storage device!"
//#define SAVEDLG_COPYING_COMPLETED		"Copying completed."
namespace Ui {
class SavefileDialog;
}

class SavefileDialog : public QDialog
{
    Q_OBJECT

public:
    void init_string();
    int step;
    QMessageBox *msgBox;	
    void init_save();	
    CMDThread   *worker;
    QThread* workerthread;	
    Utility *pUtility;
    QFile fromFile;
    QFile toFile;
    QTimer *copy_timer;
    int written;
    int check_dir_exist(char* path);
    explicit SavefileDialog(QWidget *parent = 0);
    ~SavefileDialog();
public slots:
    void onProgress();
    void onCMDdone();    
private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SavefileDialog *ui;
};

#endif // SAVEFILEDIALOG_H
