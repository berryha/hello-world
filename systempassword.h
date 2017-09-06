#ifndef SYSTEMPASSWORD_H
#define SYSTEMPASSWORD_H
#include <QWidget>
#include "global_variable.h"
namespace Ui {
class SystemPassword;
}

class SystemPassword : public QWidget
{
    Q_OBJECT

public:
    explicit SystemPassword(QWidget *parent = 0);
    ~SystemPassword();
    void SystemPasswordInit();
protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *, QEvent *);
private:
    Ui::SystemPassword *ui;
    QTimer* timer;
private slots:
    void system_passwd_confirm();
    void cancel();
public slots:
    void refresh_click();
    void hideMsg();
signals:
    void change(int);
    void display_firstdlg();
};

#endif // PASSWD_H

