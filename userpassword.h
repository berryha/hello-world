#ifndef USERPASSWORD_H
#define USERPASSWORD_H
#include <QWidget>
#include "global_variable.h"
namespace Ui {
class UserPassword;
}

class UserPassword : public QWidget
{
    Q_OBJECT

public:
    explicit UserPassword(QWidget *parent = 0);
    ~UserPassword();
    void UserPasswordInit();
protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *, QEvent *);
private:
    Ui::UserPassword *ui;
    QTimer* timer;
private slots:
    void userPasswdConfirm();
    void cancel();
public slots:
    void refresh_click();
    void hideMsg();
signals:
    void change(int);
    void displayFirstdlg();
};

#endif // PASSWD_H

