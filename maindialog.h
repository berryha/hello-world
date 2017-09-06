#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "advancedialog.h"
#include "settingdialog.h"
#include "infodialog.h"
#include "tooldialog.h"
#include "utility.h"
#include "systempassword.h"
#include "userpassword.h"
namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();
    AdvanceDialog *advancewindow;
    SettingDialog *settingDialog;
    InfoDialog *infoDialog;
    ToolDialog *toolDialog;
    SystemPassword *systemPassword;
    UserPassword *userPassword;
    void mainUiInit();
    void translateLanguage();
protected:
    bool eventFilter(QObject *target, QEvent *event);

private:
    Ui::MainDialog *ui;
    void freshSystem();
    void freshStackedTitle();
public slots:
    void on_system_pressed();
    void on_WritetoRdpconf(QString xipaddr, QString xusername, QString xpassword);

    void on_user_pressed();
    void on_info_pressed();
    void on_check_pressed();
    void on_back_pressed();

    void on_network_pressed();

    void on_update_pressed();

    void on_restore_pressed();

    void on_password_pressed();

    void on_display_pressed();

    void on_titleUser_pressed();

    void on_ping_pressed();

    void on_log_pressed();
    void systemClick(int value);
    void userClick(int value);
    void connect_click();

    void on_account_pressed();
    void on_changeLanguage(int language);
    void on_autodetectRES(int);

signals:
    void back_pressed();
    void changeLanguage(int language);
    void WritetoRdpconftoMain(QString xipaddr, QString xusername, QString xpassword);
    void m_autodetectRES(int);
};

#endif // MAINDIALOG_H
