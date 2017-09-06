#include "create_users.h"
#include "ui_create_users.h"
#include <QDebug>
#include <QRegExp>
static QSettings userinfo(userinfo_path,QSettings::IniFormat);
//static QSettings password();
//static QSettings setting();
//static QSettings usb_upgrade();
Create_Users::Create_Users(QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::Create_Users)
{
    ui->setupUi(this);
    pUtility = new Utility;
    m_userMode = "new";
    advmsg = new QMessageBox;
    advmsg->setParent(this);
    advmsg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    create_users_init();
}

Create_Users::~Create_Users()
{
    delete ui;
}

void Create_Users::create_users_init()
{
    QString UserNumber;
    UserNumber = userinfo.value("UserNumber").toString();
    if(UserNumber.isEmpty()){
        UserNumber="1";
        userinfo.setValue("UserNumber",UserNumber);
    }
}

void Create_Users::editUsers(QString connectName, QString ipAddress, QString userName, QString password, int currentEdit)
{
    ui->connect_name->setText(connectName);
    ui->IP_address->setText(ipAddress);
    ui->username->setText(userName);
    ui->password->setText(password);
    m_userMode = "edit";
    m_currentEdit = currentEdit;
}

void Create_Users::on_Cancel_clicked()
{
    this->close();
}

bool Create_Users::isConnectNamerepeat(const QString& name)
{
    QString connectNameCur = name;
    QString connectNameOri;
    for(int i = 1; i <= userinfo.value("UserNumber").toInt(); i++){
        if(m_userMode == "edit") {
            if(m_currentEdit==i)
                continue;
        }
        connectNameOri = "ConnectName" + QString::number(i,10);
        if(connectNameCur == userinfo.value(connectNameOri).toString()){
            return false;
        }

        if(connectNameCur == QString("Manual")){
            return false;
        }
    }
    return true;
}
void Create_Users::on_OK_clicked()
{
    int NUM;
    int numEdit;
    QString UserNumber;
    QString ConnectName,IPAddress,Username,Password;
    UserNumber = userinfo.value("UserNumber").toString();
    NUM = UserNumber.toInt();
    if(ui->connect_name->text().isEmpty() || ui->IP_address->text().isEmpty()){
        advmsg->setText(tr("connect name or IP address can not be empty!"));
        advmsg->exec();
        ui->connect_name->clear();
        ui->username->clear();
        ui->IP_address->clear();
        ui->username->clear();
        ui->password->clear();
        return;
    }

    //QRegExp rx("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(\\:([1-6]?[0-9]?[0-9]?[0-9]?[0-9]))?");
    QRegExp rx("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(\\:(([1-6][0-9][0-9][0-9][0-9])|([1-9][0-9][0-9][0-9])|([1-9][0-9][0-9])|([1-9]?[0-9])))?");

    if( !rx.exactMatch( ui->IP_address->text() ) )
    {
        advmsg->setText(tr("IP address is not rightful!"));
        advmsg->exec();
        return;
    }

    if(isConnectNamerepeat(ui->connect_name->text()) == false){
        advmsg->setText(tr("connect name repeat!"));
        advmsg->exec();
        ui->connect_name->clear();
        ui->username->clear();
        ui->IP_address->clear();
        ui->username->clear();
        ui->password->clear();
        return;
    }
    if(m_userMode == "new"){
        if(NUM >= 7){
            advmsg->setText(tr("userinfo list if full!"));
            advmsg->exec();
            return;
        }
        NUM = NUM + 1;
        ConnectName += "ConnectName" + UserNumber;
        IPAddress    += "IPAddress" + UserNumber;
        Username     += "Username" + UserNumber;
        Password     += "Password" + UserNumber;
        userinfo.setValue(ConnectName,ui->connect_name->text());
        userinfo.setValue(IPAddress,ui->IP_address->text());
        userinfo.setValue(Username,ui->username->text());
        userinfo.setValue(Password,ui->password->text());
        userinfo.setValue("UserNumber",QString::number(NUM,10));
    }else if(m_userMode == "edit"){
        QString numCurrent = QString::number(m_currentEdit, 10);
        ConnectName += "ConnectName" + numCurrent;
        IPAddress    += "IPAddress" + numCurrent;
        Username     += "Username" + numCurrent;
        Password     += "Password" + numCurrent;
        userinfo.setValue(ConnectName,ui->connect_name->text());
        userinfo.setValue(IPAddress,ui->IP_address->text());
        userinfo.setValue(Username,ui->username->text());
        userinfo.setValue(Password,ui->password->text());
    }
    this->close();
}
