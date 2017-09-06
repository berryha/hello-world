/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "Chance Tsai" <chance_tsai@aspeedtech.com>
 *   
 * This file is part of "connect_ui".
 *
 * "connect_ui" is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * "connect_ui" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with "connect_ui"; If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#ifndef INFODIALOG_H
#define INFODIALOG_H
#include <QDialog>
namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    void init_string();
    QString GetBuildDate();
    void translateLanguage();
    int readfromfile(const char* keyitem,char* value,const char* path);
    explicit InfoDialog(QWidget *parent = 0);
    ~InfoDialog();

private:
    Ui::InfoDialog *ui;
private slots:

signals:


};

#endif // INFODIALOG_H
