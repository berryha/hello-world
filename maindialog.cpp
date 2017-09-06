#include "maindialog.h"
#include "ui_maindialog.h"
#include <QTime>
MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    advancewindow = new AdvanceDialog(this);
    settingDialog = new SettingDialog(this);
    infoDialog = new InfoDialog(this);
    toolDialog = new ToolDialog(this);
    systemPassword = new SystemPassword(this);
    userPassword = new UserPassword();
    ui->stackedWidget->addWidget(advancewindow);
    ui->stackedWidget->addWidget(settingDialog);
    ui->stackedWidget->addWidget(infoDialog);
    ui->stackedWidget->addWidget(toolDialog);
    ui->stackedWidget->addWidget(systemPassword);
    ui->stackedWidget->addWidget(userPassword);


    ui->stackedWidget->setCurrentWidget(advancewindow);
    ui->stackedTitle->setCurrentIndex(0);

    ui->system->installEventFilter(this);
    ui->user->installEventFilter(this);
    ui->check->installEventFilter(this);
    ui->info->installEventFilter(this);

    ui->account->installEventFilter(this);
    ui->network->installEventFilter(this);
    ui->password->installEventFilter(this);
    ui->restore->installEventFilter(this);
    ui->update->installEventFilter(this);

    ui->display->installEventFilter(this);
    ui->titleUser->installEventFilter(this);
    ui->ping->installEventFilter(this);
    ui->log->installEventFilter(this);
    connect(systemPassword,SIGNAL(change(int)),this,SLOT(systemClick(int)));
    connect(systemPassword,SIGNAL(display_firstdlg()),this,SLOT(connect_click()));
    connect(userPassword,SIGNAL(change(int)),this,SLOT(userClick(int)));
    connect(userPassword,SIGNAL(displayFirstdlg()),this,SLOT(connect_click()));
    connect(settingDialog,SIGNAL(changeLanguage(int)),this,SLOT(on_changeLanguage(int)));
    connect(settingDialog,SIGNAL(autodetectRES(int)),this,SLOT(on_autodetectRES(int)));

    connect(advancewindow,SIGNAL(WritetoRdpconf(QString, QString, QString)),this,SLOT(on_WritetoRdpconf(QString, QString, QString)));

    mainUiInit();
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::mainUiInit()
{
    freshSystem();
    freshStackedTitle();
    ui->system->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	outline:none; color: rgb(175, 210, 255);\n""}\n""\n"""));  
    ui->titleUser->hide();
}

void MainDialog::on_system_pressed()
{
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString system_passwd = settings.value("system_password").toString();
    ui->stackedWidget->setCurrentWidget(advancewindow);
    ui->stackedTitle->setCurrentIndex(0);

    if(system_passwd.isEmpty()) //若不存在密码信息文件
    {
        ui->account->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));

        advancewindow->on_advanceTab_currentChanged(0);

    }else {
        systemPassword->SystemPasswordInit();
        ui->account->setEnabled(false);
        ui->network->setEnabled(false);
        ui->update->setEnabled(false);
        ui->restore->setEnabled(false);
        ui->password->setEnabled(false);
        ui->account->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n"""));
        ui->stackedWidget->setCurrentWidget(systemPassword);
    }
    ui->system->show();
    ui->user->show();
    ui->check->hide();
    ui->info->hide();

}

void MainDialog::on_user_pressed()
{
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString system_passwd = settings.value("user_password").toString();
    ui->stackedWidget->setCurrentWidget(settingDialog);
    ui->stackedTitle->setCurrentIndex(1);

    if(system_passwd.isEmpty()) //若不存在密码信息文件
    {
     settingDialog->on_tabWidget_currentChanged(0);
    }else {
        userPassword->UserPasswordInit();
        ui->display->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n"""));
        ui->display->setEnabled(false);
        ui->titleUser->setEnabled(false);
        ui->stackedWidget->setCurrentWidget(userPassword);
    }
    ui->system->show();
    ui->user->show();
    ui->check->hide();
    ui->info->hide();

}

void MainDialog::on_check_pressed()
{
    ui->stackedWidget->setCurrentWidget(toolDialog);
    ui->stackedTitle->setCurrentIndex(2);
    toolDialog->on_checkstackedWidget_currentChanged(0);
    ui->system->hide();
    ui->user->hide();
    ui->check->show();
    ui->info->show();
    ui->check->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	outline:none; color: rgb(175, 210, 255);\n""}\n""\n"""));
}

void MainDialog::on_info_pressed()
{
    ui->stackedWidget->setCurrentWidget(infoDialog);
    ui->stackedTitle->setCurrentIndex(3);
    ui->system->hide();
    ui->user->hide();
    ui->check->show();
    ui->info->show();
    infoDialog->init_string();
}

void MainDialog::on_back_pressed()
{
    emit back_pressed();
    this->close();
}

void MainDialog::on_account_pressed()
{
    advancewindow->on_advanceTab_currentChanged(0);
}

void MainDialog::on_network_pressed()
{
    advancewindow->on_advanceTab_currentChanged(1);
}

void MainDialog::on_update_pressed()
{
    advancewindow->on_advanceTab_currentChanged(2);
    advancewindow->initial_FWUpdate();
}

void MainDialog::on_restore_pressed()
{
    advancewindow->on_advanceTab_currentChanged(3);
}

void MainDialog::on_password_pressed()
{
    advancewindow->on_advanceTab_currentChanged(4);
}

void MainDialog::on_display_pressed()
{
    settingDialog->on_tabWidget_currentChanged(0);
}

void MainDialog::on_titleUser_pressed()
{
    settingDialog->on_tabWidget_currentChanged(1);
}

void MainDialog::on_ping_pressed()
{
    toolDialog->on_checkstackedWidget_currentChanged(0);
}

void MainDialog::on_log_pressed()
{
     toolDialog->on_checkstackedWidget_currentChanged(1);
     toolDialog->initial_log();
}


void MainDialog::systemClick(int value)
{
    if(value == 1){
        ui->stackedWidget->setCurrentWidget(advancewindow);
        ui->stackedTitle->setCurrentIndex(0);
        advancewindow->on_advanceTab_currentChanged(0);
        ui->account->setEnabled(true);
        ui->display->setEnabled(true);
        ui->network->setEnabled(true);
        ui->update->setEnabled(true);
        ui->restore->setEnabled(true);
        ui->password->setEnabled(true);
        ui->account->setFocus();
    }
}

void MainDialog::userClick(int value)
{
    if(value == 1){
        ui->stackedWidget->setCurrentWidget(settingDialog);
        ui->stackedTitle->setCurrentIndex(1);
        settingDialog->on_tabWidget_currentChanged(0);
        ui->display->setEnabled(true);
        ui->titleUser->setEnabled(true);
        ui->display->setFocus();
    }

}

void MainDialog::connect_click()
{
    emit back_pressed();
    this->close();
}

void MainDialog::freshSystem()
{
        //color: rgb(175, 210, 255);
    ui->system->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/system_normal.png);\n""	color: rgb(175, 210, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->user->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/system_normal.png);\n""	color: rgb(175, 210, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->check->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/system_normal.png);\n""	color: rgb(175, 210, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->info->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/system_normal.png);\n""	color: rgb(175, 210, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
}

void MainDialog::freshStackedTitle()
{
    ui->account->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->network->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->update->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->restore->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));

    ui->password->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->display->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));

    ui->titleUser->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->ping->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
    ui->log->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel_normal.png);\n""	color: rgb(255, 255, 255);\n""}\n""QPushButton:hover{\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
}

bool MainDialog::eventFilter(QObject *target, QEvent *event)
{
    //system user check info click
    if(target == ui->system){
         if(event->type() == QEvent::FocusIn){
            freshSystem();
            freshStackedTitle();
            ui->system->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	 outline:none; color: rgb(5, 106, 255);\n""}\n""\n"""));
            ui->account->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->user){
        if(event->type() == QEvent::FocusIn){
            freshSystem();
            freshStackedTitle();
            ui->user->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	 outline:none; color: rgb(5, 106, 255);\n""}\n""\n"""));
            ui->display->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->check){
        if(event->type() == QEvent::FocusIn){
            freshSystem();
            freshStackedTitle();
            ui->check->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	 outline:none; color: rgb(5, 106, 255);\n""}\n""\n"""));
            ui->ping->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->info){
        if(event->type() == QEvent::FocusIn){
            freshSystem();
            freshStackedTitle();
            ui->info->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/button_pressed.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }

    //system tab click
    if(target == ui->account){
         if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->account->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->network){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->network->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->update){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->update->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->restore){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->restore->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->password){
         if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->password->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    //user tab click
    if(target == ui->display){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->display->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->titleUser){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->titleUser->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    //check tab
    if(target == ui->ping){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->ping->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    if(target == ui->log){
        if(event->type() == QEvent::FocusIn){
            freshStackedTitle();
            ui->log->setStyleSheet(QString::fromUtf8("QPushButton{\n""	border-image: url(:/images/images/panel.png);\n""	outline:none; color: rgb(5, 106, 183);\n""}\n"""));
        }
    }
    return false;
}

void MainDialog::on_changeLanguage(int language)
{
    qDebug()<<"MainDialog emit changeLanguage signal: "<<language;
    emit changeLanguage(language);
}

void MainDialog::translateLanguage()
{
    ui->retranslateUi(this);
    settingDialog->translateLanguage();
    advancewindow->translateLanguage();
    toolDialog->translateLanguage();
    infoDialog->translateLanguage();
}

void MainDialog::on_WritetoRdpconf(QString xipaddr, QString xusername, QString xpassword)
{
    emit WritetoRdpconftoMain(xipaddr, xusername, xpassword);
}

void MainDialog::on_autodetectRES(int index)
{
    emit m_autodetectRES(index);
}
