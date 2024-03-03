/* 
   Модуль настроек охранно-пожарной сигнализации с управлением через MQTT и Telegram
   --------------------------
   (с) 2021-2024 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#ifndef __SECURITY_H__
#define __SECURITY_H__

#include "reRx433.h"
#include "reAlarm.h"

#ifdef __cplusplus
extern "C" {
#endif

void alarmStart();

#ifdef __cplusplus
}
#endif

#endif // __SECURITY_H__