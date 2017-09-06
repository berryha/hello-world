#include "reconnect.h"
#include "ui_reconnect.h"

Reconnect::Reconnect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Reconnect)
{
    ui->setupUi(this);
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(100);


    qtimer = new QTimer();
    temp = 100;
    connect(qtimer, SIGNAL(timeout()), this, SLOT(hideMsg()));
    connect(ui->ok,SIGNAL(clicked()),this,SLOT(ok_click()));
    connect(ui->cancel,SIGNAL(clicked()),this,SLOT(cancel_click()));
    ui->reconnect_information->setText(tr("Auto reconnetion after ten seconds"));
}

Reconnect::~Reconnect()
{
    delete ui;
}


void Reconnect::ok_click()
{
    QApplication::quit();
}

void Reconnect::hideMsg()
{
    --temp;
    ui->progressBar->setValue(temp);

    if(temp == 0)
    {
        qtimer->stop();
        temp = 100;
        QApplication::quit();
    }
}

void Reconnect::cancel_click()
{
    qtimer->stop();
    this->close();
}


void Reconnect::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return){
        ok_click();
    }else if( keyEvent->key() == Qt::Key_Escape){
        cancel_click();
    }
}
