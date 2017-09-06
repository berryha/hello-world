#ifndef BOOTPASSWORD_H
#define BOOTPASSWORD_H

#include <QWidget>
#include "global_variable.h"
namespace Ui {
class BootPassword;
}

class BootPassword : public QWidget
{
    Q_OBJECT
    
public:
    explicit BootPassword(QWidget *parent = 0);
    ~BootPassword();
    
private:
    Ui::BootPassword *ui;
     QTimer* timer;
protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *, QEvent *);
private slots:
    void boot_passwd_confirm();
    void cancel();
public slots:
    void refresh_click();
    void hideMsg();
};

#endif // BOOTPASSWORD_H
