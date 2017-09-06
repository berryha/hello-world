#ifndef CREATE_USERS_H
#define CREATE_USERS_H

#include <QDialog>
#include "utility.h"
namespace Ui {
class Create_Users;
}

class Create_Users : public QDialog
{
    Q_OBJECT
    
public:
    explicit Create_Users(QWidget *parent = 0);
    ~Create_Users();
    Utility *pUtility;
    QMessageBox *advmsg;
    void create_users_init();
    void editUsers(QString connectName, QString ipAddress, QString userName, QString password, int currentEdit);
private slots:
    void on_Cancel_clicked();
    bool isConnectNamerepeat(const QString& name);
    void on_OK_clicked();

private:
    Ui::Create_Users *ui;
    QString m_userMode;
    int m_currentEdit;
};

#endif // CREATE_USERS_H
