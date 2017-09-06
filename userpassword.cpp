#include "userpassword.h"
#include "ui_userpassword.h"
#include <QSettings>
#include <QDebug>
#include <QTimer>


UserPassword::UserPassword(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserPassword)
{

    ui->setupUi(this);
    timer = new QTimer();
    ui->password->installEventFilter(this);
    connect(ui->ok,SIGNAL(clicked()),this,SLOT(userPasswdConfirm()));
    connect(ui->cancel,SIGNAL(clicked()),this,SLOT(cancel()));
    connect(timer, SIGNAL(timeout()), this, SLOT(hideMsg()));
    /*去掉LineEdit右键菜单*/
    ui->password->setContextMenuPolicy(Qt::NoContextMenu);

}

UserPassword::~UserPassword()
{
    delete ui;
    if (NULL != timer)//释放timer定时器
    {
       delete timer;
       timer = NULL;
    }
}

void UserPassword::UserPasswordInit()
{
    ui->password->clear();
}

void UserPassword::keyPressEvent(QKeyEvent *keyEvent)
{
    if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return){
        userPasswdConfirm();
        ui->ok->setFocus();
    }else if (keyEvent->key() == Qt::Key_CapsLock){
        int m_kb_fd = open("/dev/tty0",O_NOCTTY|O_ASYNC);
        unsigned int m_KBD_status;
        QString path="/usr/ast/userinfo.conf";
        QSettings settings(path,QSettings::IniFormat);
        ioctl(m_kb_fd,KDGETLED,&m_KBD_status);
        settings.setValue("m_KBD_status",m_KBD_status & CapsLock_ON);
        if((m_KBD_status & CapsLock_ON) == 0){
            ui->user_information->setText(tr("CapsLock OFF"));
        }else if((m_KBD_status & CapsLock_ON) == 4){
            ui->user_information->setText(tr("CapsLock ON"));
        }
        timer->start(3000);
    }
}

bool UserPassword::eventFilter(QObject *object, QEvent *event)
{
    QString path="/usr/ast/userinfo.conf";
    QSettings settings(path,QSettings::IniFormat);
    if(settings.value("m_KBD_status").toInt() == 4)
        {
        if (object == ui->password)
            {
                if( event->type() == QEvent::KeyPress)
                {
                    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                    if ( (keyEvent->key() >= Qt::Key_A) && (keyEvent->key() <= Qt::Key_Z) )
                    {
                        if(keyEvent->modifiers() & Qt::ShiftModifier)
                        {
                             keyEvent=new QKeyEvent(QEvent::KeyPress,keyEvent->key()+0x20,Qt::NoModifier,keyEvent->text().toLower().at(0),false,1);
                              qApp->postEvent(QApplication::focusWidget(), keyEvent);
                             return true;
                        }
                        else
                        {
                             if(keyEvent->key()!=*(keyEvent->text().toLatin1().data()))
                             {
                                keyEvent=new QKeyEvent(QEvent::KeyPress,keyEvent->key(),Qt::NoModifier,keyEvent->text().toUpper().at(0),false,1);
                                qApp->postEvent(QApplication::focusWidget(), keyEvent);
                                 return true;
                             }
                        }
                    }
                }
            }
        }
    return false;
}

void UserPassword::userPasswdConfirm()
{
    QString path="/usr/ast/password.conf";
    QSettings settings(path,QSettings::IniFormat);
    QString system_passwd = settings.value("system_password").toString();
    if(ui->password->text() == system_passwd){
        emit change(1);
    }else{
        ui->user_information->setText(tr("Password is incorrect"));
        timer->start(2000);  //触发计时器，3秒过后将会触发hideMsg()
        ui->password->clear();
    }
}

void UserPassword::cancel()
{
    ui->password->clear();
    //emit displayFirstdlg();
}

void UserPassword::refresh_click()
{
    ui->password->clear();
}

void UserPassword::hideMsg()
{
    ui->user_information->clear();
}
