#include "progressdialog.h"
#include "ui_progressdialog.h"

progressDialog::progressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::progressDialog)
{
    ui->setupUi(this);
    initdata();
}

progressDialog::~progressDialog()
{
    delete ui;
}
void progressDialog::onStop()
{
    int max_time=MAX_DEFAULT_WAIT_TIME;
    if(waittime > 0)
    {
        max_time=waittime; //caller send in
    }	
    m_timer->stop();
    ui->progressBar->setValue(max_time);
    stop_timer->singleShot(100, this,SLOT(onexit()));
}

void progressDialog::onexit()
{
    progressDialog::close();
}
void progressDialog::keyPressEvent(QKeyEvent * event)
{
    if(event->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(event);
    }
}
void progressDialog::timer_timeout()
{
    int max_time=MAX_DEFAULT_WAIT_TIME;
    going++;
    if(waittime > 0)
    {
        max_time=waittime; //caller send in
    }
    if(going==max_time)
    {
        going=0;
    }
    ui->progressBar->setValue(going);
    //qDebug("max:%d",ui->progressBar->maximum());
}
void progressDialog::set_MAX_WAIT_TIME(int wtime)
{
	//qDebug("settime:%d",wtime);
    waittime=wtime;     
    if(waittime > 0)
    {
        max_time=waittime; //caller send in
    }       
    ui->progressBar->setMaximum(max_time);    
}

void progressDialog::initdata()
{
    max_time=MAX_DEFAULT_WAIT_TIME;	
    waittime=0;
    going=0;
    //waittime=0;
    stop_timer=new QTimer(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(timer_timeout()));
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    //qDebug("waittime=%d",waittime);
    if(waittime > 0)
    {
        max_time=waittime; //caller send in
    }       
    ui->progressBar->setMaximum(max_time);
    m_timer->start(500);
    /*
    workerthread = new QThread;
    worker = new dhcp_thread;
    //connect(worker, SIGNAL(changestat(QString)),ui->DHCP_IP,SLOT(setText(QString)));
    connect(workerthread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(finished()), workerthread, SLOT(quit()));
    connect(workerthread, SIGNAL(finished()),SLOT(onDHCPdone()));
    worker->moveToThread(workerthread);
    //qApp->setOverrideCursor(Qt::WaitCursor);
    //workerthread->start();
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(MAX_WAIT_TIME);
    going=0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(timer_timeout()));
    //connect(worker, SIGNAL(finished()),m_timer,SLOT(stop()));
    m_timer->start(500);
    thread_timer= new QTimer(this);
    thread_timer->singleShot(0, this,SLOT(thread_start()));
    */
}
