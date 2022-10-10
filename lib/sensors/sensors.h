#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "project_config.h"
#include "def_consts.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "reLed.h" 
#include "reRangeMonitor.h"
#include "reSensor.h" 
#include "reDHTxx.h"
#include "reHTU2x.h"
#include "reDS18x20.h"

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- Сенсоры -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// BME280: Улица
#define SENSOR_OUTDOOR_NAME "Улица (AM2302)"
#define SENSOR_OUTDOOR_KEY "out"
#define SENSOR_OUTDOOR_TOPIC "outdoor"
#define SENSOR_OUTDOOR_FILTER_MODE SENSOR_FILTER_AVERAGE
#define SENSOR_OUTDOOR_FILTER_SIZE 20
#define SENSOR_OUTDOOR_ERRORS_LIMIT 5

static DHTxx sensorOutdoor(1);

// HTU21: Комната
#define SENSOR_INDOOR_NAME "Дом (SHT20)"
#define SENSOR_INDOOR_KEY "in"
#define SENSOR_INDOOR_BUS 0
#define SENSOR_INDOOR_ADDRESS HTU2X_ADDRESS
#define SENSOR_INDOOR_TOPIC "indoor"
#define SENSOR_INDOOR_FILTER_MODE SENSOR_FILTER_RAW
#define SENSOR_INDOOR_FILTER_SIZE 0
#define SENSOR_INDOOR_ERRORS_LIMIT 3

static HTU2x sensorIndoor(2);

// DS18B20: Теплоноситель
#define SENSOR_BOILER_NAME "Котёл (DS18B20)"
#define SENSOR_BOILER_KEY "bo"
#define SENSOR_BOILER_TOPIC "boiler"
#define SENSOR_BOILER_FILTER_MODE SENSOR_FILTER_RAW
#define SENSOR_BOILER_FILTER_SIZE 0
#define SENSOR_BOILER_ERRORS_LIMIT 3

static DS18x20 sensorBoiler(3);

// Период публикации данных с сенсоров на MQTT
static uint32_t iMqttPubInterval = CONFIG_MQTT_SENSORS_SEND_INTERVAL;
// Период публикации данных с сенсоров на OpenMon
#if CONFIG_OPENMON_ENABLE
static uint32_t iOpenMonInterval = CONFIG_OPENMON_SEND_INTERVAL;
#endif // CONFIG_OPENMON_ENABLE
// Период публикации данных с сенсоров на NarodMon
#if CONFIG_NARODMON_ENABLE
static uint32_t iNarodMonInterval = CONFIG_NARODMON_SEND_INTERVAL;
#endif // CONFIG_NARODMON_ENABLE
// Период публикации данных с сенсоров на ThingSpeak
#if CONFIG_THINGSPEAK_ENABLE
static uint32_t iThingSpeakInterval = CONFIG_THINGSPEAK_SEND_INTERVAL;
#endif // CONFIG_THINGSPEAK_ENABLE

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------- Контроль температуры в доме ---------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

#define CONTROL_TEMP_GROUP_KEY                "tmon"
#define CONTROL_TEMP_GROUP_TOPIC              "temp_control"
#define CONTROL_TEMP_GROUP_FRIENDLY           "Контроль температуры"
#define CONTROL_TEMP_LOCAL                    false
#define CONTROL_TEMP_QOS                      2
#define CONTROL_TEMP_RETAINED                 1


#define CONTROL_TEMP_INDOOR_KEY               "indoor"
#define CONTROL_TEMP_INDOOR_TOPIC             "indoor"
#define CONTROL_TEMP_INDOOR_FRIENDLY          "Дом"

#define CONTROL_TEMP_INDOOR_NOTIFY_KIND       MK_MAIN
#define CONTROL_TEMP_INDOOR_NOTIFY_PRIORITY   MP_CRITICAL
#define CONTROL_TEMP_INDOOR_NOTIFY_ALARM      1
#define CONTROL_TEMP_INDOOR_NOTIFY_TOO_LOW    "❄️ Температура в доме <i><b>слишком низкая</b></i>: <b>%.2f</b> °С"
#define CONTROL_TEMP_INDOOR_NOTIFY_TOO_HIGH   "☀️ Температура в доме <i><b>слишком высокая</b></i>: <b>%.2f</b> °С"
#define CONTROL_TEMP_INDOOR_NOTIFY_NORMAL     "🆗 Температура в доме <i><b>вернулась в нормальный диапазон</b></i>: <b>%.2f</b> °С"

static reRangeMonitor tempMonitorIndoor(20, 30, 0.1, nullptr, nullptr, nullptr);

#define CONTROL_TEMP_BOILER_KEY               "boiler"
#define CONTROL_TEMP_BOILER_TOPIC             "boiler"
#define CONTROL_TEMP_BOILER_FRIENDLY          "Котёл"

#define CONTROL_TEMP_BOILER_NOTIFY_KIND       MK_MAIN
#define CONTROL_TEMP_BOILER_NOTIFY_PRIORITY   MP_CRITICAL
#define CONTROL_TEMP_BOILER_NOTIFY_ALARM      1
#define CONTROL_TEMP_BOILER_NOTIFY_TOO_LOW    "❄️ Температура теплоносителя <i><b>слишком низкая</b></i>: <b>%.2f</b> °С"
#define CONTROL_TEMP_BOILER_NOTIFY_TOO_HIGH   "☀️ Температура теплоносителя <i><b>слишком высокая</b></i>: <b>%.2f</b> °С"
#define CONTROL_TEMP_BOILER_NOTIFY_NORMAL     "🆗 Температура теплоносителя <i><b>вернулась в нормальный диапазон</b></i>: <b>%.2f</b> °С"

static reRangeMonitor tempMonitorBoiler(25, 80, 1.0, nullptr, nullptr, nullptr);

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- Задача --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool sensorsTaskStart();
bool sensorsTaskSuspend();
bool sensorsTaskResume();

#endif // __SENSORS_H__