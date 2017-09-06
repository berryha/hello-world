#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QKeyEvent>
#define MAX_DEFAULT_WAIT_TIME 30
namespace Ui {
class progressDialog;
}

class progressDialog : public QDialog
{
    Q_OBJECT

public:
	int max_time;	
    int waittime;
    int going;
    QTimer *m_timer;
    QTimer *stop_timer;
    void initdata();
    //QThread* workerthread;
    void set_MAX_WAIT_TIME(int wtime);
    explicit progressDialog(QWidget *parent = 0);
    ~progressDialog();
private slots:
    void onStop();
    void timer_timeout();
    void onexit();
protected:
virtual void keyPressEvent(QKeyEvent * event);
private:
    Ui::progressDialog *ui;
//signals:

};

#endif // PROGRESSDIALOG_H
