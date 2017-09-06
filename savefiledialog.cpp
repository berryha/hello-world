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
#include "savefiledialog.h"
#include "ui_savefiledialog.h"

SavefileDialog::SavefileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SavefileDialog)
{
    char savepath[MAX_SAVE_PATH_LENGTH];
    pUtility = new Utility;
    //char foldername[128];
    ui->setupUi(this);    
    //initial combobox
    DIR* dir;
    struct dirent *dent;
    struct stat st;	
    init_string();
    dir= opendir(SAVE_ROOT_PATH);
    if (dir)
	{
		while ((dent = readdir(dir)) != NULL)
		{
			//memset(foldername,0,128);			
			//printf("foldername %s\n",dent->d_name);

			//avoid display . and ..
			if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			    continue;	
		        //only display directory, but file
			memset(savepath,0,MAX_SAVE_PATH_LENGTH);
			sprintf(savepath,"%s%s",SAVE_ROOT_PATH,dent->d_name);
			if(stat(savepath, &st) == -1)
			{
				PRINT_LOG((LOG_ERR,"stat error:%s\n",strerror(errno)));
				continue;
			}	  
			if(S_ISDIR(st.st_mode))
			{
				//add folder path to combobox
				ui->savecomboBox->addItem(savepath);
			}
		}	
		closedir(dir);
		//disable save button without usb
		if(ui->savecomboBox->count()==0)
		{
            ui->saveButton->setEnabled(false);
		}		
	}
	else
	{
		PRINT_LOG((LOG_ERR,"opendir error:%s\n",strerror(errno)));
	}	
	ui->progressBar->setVisible(false);
	ui->progressBar->setValue(0);
	ui->filenamelineEdit->setMaxLength(16);
	
    msgBox = new QMessageBox;
    msgBox->setParent(this);
    init_save();
	copy_timer = new QTimer(this);
	connect(copy_timer, SIGNAL(timeout()), this, SLOT(onProgress()));

}
void SavefileDialog::init_string()
{		
}
void SavefileDialog::init_save()
{
    workerthread = new QThread;
    worker = new CMDThread;
    //connect(worker, SIGNAL(changestat(QString)),ui->pingresulttextEdit,SLOT(append(QString)));
    connect(workerthread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), workerthread, SLOT(quit()));
    connect(workerthread, SIGNAL(finished()),SLOT(onCMDdone()));
    worker->moveToThread(workerthread);		
}
SavefileDialog::~SavefileDialog()
{
    delete ui;
}

void SavefileDialog::on_saveButton_clicked()
{
	//FILE * tmpfile;
	QString msg;
	QString savestr;
    QString filename;
    //int filesize=0;
    //int tofilesize=0;
    char cmdline[MAX_COMMAND_LEN];
	
	savestr=ui->savecomboBox->currentText();	
	//check user didn't plugout usb
    memset(cmdline,0,MAX_COMMAND_LEN);
    if(pUtility->readfromfile("MESSAGE_WARNING_TITLE",cmdline,UI_STRING_PATH))
    {
        msgBox->setWindowTitle(QString(cmdline));
    }
    else
    {
        msgBox->setWindowTitle(tr("Warning"));
    }
	if(check_dir_exist(savestr.toLatin1().data()))
	{	
		//copy file to selected USB
		fromFile.setFileName(TMP_LOG_PATH);
		filename=ui->filenamelineEdit->text();
		if(filename.length()==0)
		{		
            msgBox->setText(tr("Please input filename!"));
		    msgBox->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
		    if(msgBox->exec()==QMessageBox::Ok)
		    {
			ui->filenamelineEdit->setFocus();
		    }
		    return;
		}
		savestr+="/";
		savestr+=ui->filenamelineEdit->text();//"/eventlog.txt";
		toFile.setFileName(savestr);
		if(!toFile.exists())
		{
			ui->progressBar->setValue(0);
			ui->progressBar->setMinimum(0);
			ui->progressBar->setMaximum( MAX_PROGRESS_STEP);
			memset(cmdline,0,MAX_COMMAND_LEN);
			sprintf(cmdline,"cp %s %s",TMP_LOG_PATH,savestr.toLatin1().data());
			init_save();
			worker->setcmdline(cmdline);
			workerthread->start();
			step=0;
			ui->progressBar->setVisible(true);
			ui->saveButton->setEnabled(false);
			ui->cancelButton->setEnabled(false);
			ui->savecomboBox->setEnabled(false);
			ui->filenamelineEdit->setEnabled(false);
			copy_timer->start(1000);
		}
		else
        {
            msgBox->setText(tr("A duplicate file name has existed! Please rename your file."));
			msgBox->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
			if(msgBox->exec()==QMessageBox::Ok)
			{
				ui->filenamelineEdit->setFocus();
			}
		}
	}
	else
	{		
        msgBox->setText(tr("Please plugin usb mass storage device!"));
		msgBox->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
		msgBox->show();
	}	
}
int SavefileDialog::check_dir_exist(char* path)
{
	DIR* dir = opendir(path);
	if (dir)
	{
	    //dir exists.	    
	    closedir(dir);	
	    return 1;
	}
	else
	{
	    PRINT_LOG((LOG_ERR,"check dir exist:[%s]\n", strerror(errno)));
	}	
	return 0;
}

void SavefileDialog::onProgress()
{
       step++;
       if(step==MAX_PROGRESS_STEP)
       {
       	       step=0;
       }
       ui->progressBar->setValue(step);       
}

void SavefileDialog::onCMDdone()
{
	char cmdline[MAX_COMMAND_LEN];
	copy_timer->stop();
	ui->progressBar->setValue(MAX_PROGRESS_STEP);
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
	system("sync");
    msgBox->setWindowTitle(tr("Information"));
    msgBox->setText(tr("Copying completed."));
    msgBox->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    if(msgBox->exec()==QMessageBox::Ok)
    {
        SavefileDialog::close();
    }
	ui->progressBar->setVisible(false);			
}

void SavefileDialog::on_cancelButton_clicked()
{
    this->close();
}
