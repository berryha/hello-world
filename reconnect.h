#ifndef RECONNECT_H
#define RECONNECT_H

#include <QDialog>
#include <QTimer>
#include <QKeyEvent>

namespace Ui
{
    class Reconnect;
}


class Reconnect
        : public QDialog
{
    Q_OBJECT
    
public:
    explicit Reconnect(QWidget *parent = 0);
    QTimer* qtimer;
    ~Reconnect();

protected:
    void keyPressEvent(QKeyEvent *);

private:
    Ui::Reconnect *ui;
    int temp;

private slots:
    void hideMsg();
    void ok_click();
    void cancel_click();
};

#endif // RECONNECT_H
