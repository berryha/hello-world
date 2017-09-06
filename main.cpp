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
#include <QApplication>
#include "mainwindow.h"
#include "bootpassword.h"
#include "reconnect.h"
#include <QTranslator>
#include <QWSServer>

static QSettings userinfo(userinfo_path,QSettings::IniFormat);
static QSettings settings(setting_path,QSettings::IniFormat);
static QSettings passwords(password_path,QSettings::IniFormat);
int current_language;

int main(int argc, char *argv[])
{	
    QApplication a(argc, argv);
    MainWindow w;

#ifdef __i386__

#else
    QWSServer::setBackground(QColor(0,0,0,0));
#endif

//    QWSServer::setBackground(QBrush(QImage("/usr/ast/logo.png")));

//    #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
//            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//            QTextCodec::setCodecForLocale(codec);
//            QTextCodec::setCodecForCStrings(codec);
//            QTextCodec::setCodecForTr(codec);
//    #endif

    QTranslator *m_pTranslator;
    m_pTranslator = new QTranslator;
    if(settings.value("LANGUAGE").toString().isEmpty() || settings.value("LANGUAGE").toInt() == 0){
        current_language = 0;
        m_pTranslator->load("/usr/ast/cn.qm");
    }else{
        current_language = 1;
        m_pTranslator->load("/usr/ast/en.qm");
    }
    qApp->installTranslator(m_pTranslator);
    w.setTranslator(m_pTranslator, current_language);

    BootPassword bootPassword;
    Reconnect r;
    if(userinfo.value("ContinueLogin").toInt() == 1){
        r.setGeometry(312,284,400,200);
        r.setModal(true);
        r.setWindowTitle("reconnection");
        r.show();
        r.qtimer->start(100);
    }
    w.show();
    if(!passwords.value("boot_password").toString().isEmpty()){
        bootPassword.show();
    }
    return a.exec();
}
