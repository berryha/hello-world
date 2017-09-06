#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QKeyEvent>
#include <QSettings>
#define CapsLock_ON                     (1<<2)
#define NumLock_ON                      (1<<1)

#define userinfo_path               "/usr/ast/userinfo.conf"
#define password_path               "/usr/ast/password.conf"
#define setting_path                "/usr/ast/setting.conf"
#define usb_upgrade_path            "/usr/ast/usb_upgrade.conf"
#endif // GLOBAL_VARIABLE_H



