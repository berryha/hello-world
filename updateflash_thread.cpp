/*******************************************************************************
 * Copyright (c) 2016 Aspeed Technology Inc. <http://www.ASPEEDtech.com>
 *
 * Author: "William Lin" <william_lin@aspeedtech.com>
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
#include "updateflash_thread.h"

updateflash_thread::updateflash_thread(QObject *parent) : QObject(parent){

}

void updateflash_thread::run(){

    FILE *hopen;
    FILE *chkupdate_open;
    int mtd_number;
    char cmdline[256];
    char total_mtd[10];
    char chkupdate[50];
    char updatedone[] = "All done";

    //Get the total mtd number
    memset(cmdline, 0, 256);
    sprintf(cmdline,"cat /proc/mtd | grep mtd -c");
    //qDebug()<<"cmdline="+cmdline;
    hopen=popen(cmdline,"r");
    while(fgets(total_mtd, sizeof(total_mtd) - 1, hopen) != NULL){}
    mtd_number = atoi(total_mtd);
    qDebug("mtdnumber = %d", mtd_number);

    //Create a tmp file
    memset(cmdline, 0, 256);
    sprintf(cmdline,"echo '==firmware update log==' >> /tmp/fwupdatelog");
    popen(cmdline,"r");

    while(1){
        //Continue check the update status(need to check dd and flash_erase action)
        memset(cmdline, 0, 256);
        sprintf(cmdline,"cat /tmp/fwupdatelog | busybox tail -1");
        chkupdate_open = popen(cmdline,"r");
        if(chkupdate_open != NULL)
            while(fgets(chkupdate, sizeof(chkupdate) - 1, chkupdate_open) != NULL){}

        qDebug("Firmware update status : %s", chkupdate);
        //qDebug("Comapre : %d", strcmp(chkupdate, updatedone));

        if(strcmp(chkupdate, updatedone) == 10){//break the while
            break;
        }else if(strcmp(chkupdate, updatedone) != 0){//show the current action
            //ui->UpdateActionNameLabel->setText(chkupdate);
            sleep(2);
        }else{
            //unknow satus
            sleep(2);
        }

        fclose(chkupdate_open);
    }//End of while

    //Update is done, change safe_mode back to 0, and wait for 3s for flash write done.
    system("fw_setenv safe_mode 0");

    //Do update action
    emit finished();
}
