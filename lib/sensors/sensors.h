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
#include "reBME280.h"
#include "reDS18x20.h"

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- Сенсоры -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// DHT22: Улица
#define SENSOR_OUTDOOR_NAME "Улица (AM2302)"
#define SENSOR_OUTDOOR_KEY "out"
#define SENSOR_OUTDOOR_TOPIC "outdoor"
#define SENSOR_OUTDOOR_FILTER_MODE SENSOR_FILTER_AVERAGE
#define SENSOR_OUTDOOR_FILTER_SIZE 20
#define SENSOR_OUTDOOR_ERRORS_LIMIT 3

static DHTxx sensorOutdoor(1);

// BME280: Комната
#define SENSOR_INDOOR_NAME "Комната (BME280)"
#define SENSOR_INDOOR_KEY "in"
#define SENSOR_INDOOR_BUS 0
#define SENSOR_INDOOR_ADDRESS BME280_ADDRESS_0X76
#define SENSOR_INDOOR_TOPIC "indoor"
#define SENSOR_INDOOR_FILTER_MODE SENSOR_FILTER_RAW
#define SENSOR_INDOOR_FILTER_SIZE 0
#define SENSOR_INDOOR_ERRORS_LIMIT 10

static BME280 sensorIndoor(2);

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
// ------------------------------------------------------ Термостат ------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// Режимы работы термостата
typedef enum {
  THERMOSTAT_OFF = 0,       // Котел выключен всегда
  THERMOSTAT_ON,            // Котел включен всегда (без учета расписания и температуры)
  THERMOSTAT_TIME,          // Только управление по расписанию (без учета температуры)
  THERMOSTAT_TEMP,          // Только управление по температуре (без учета расписания)
  THERMOSTAT_TIME_AND_TEMP  // Управление по расписанию и температуре одновременно
} thermostat_mode_t;

// Параметры регулирования температуры в доме
static float thermostatInternalTemp = 22.0;
static float thermostatInternalHyst = 1.0;
static timespan_t thermostatTimespan = 15000800U;
static thermostat_mode_t thermostatMode = THERMOSTAT_TIME_AND_TEMP;
static bool thermostatNotify = true;

#define CONTROL_THERMOSTAT_GROUP_KEY              "ths"
#define CONTROL_THERMOSTAT_GROUP_TOPIC            "thermostat"
#define CONTROL_THERMOSTAT_GROUP_FRIENDLY         "Термостат"

#define CONTROL_THERMOSTAT_LOCAL                  false
#define CONTROL_THERMOSTAT_QOS                    1
#define CONTROL_THERMOSTAT_RETAINED               1

#define CONTROL_THERMOSTAT_PARAM_TEMP_KEY         "temperature"
#define CONTROL_THERMOSTAT_PARAM_TEMP_FRIENDLY    "Температура"
#define CONTROL_THERMOSTAT_PARAM_HYST_KEY         "hysteresis"
#define CONTROL_THERMOSTAT_PARAM_HYST_FRIENDLY    "Гистерезис"
#define CONTROL_THERMOSTAT_PARAM_TIME_KEY         "timespan"
#define CONTROL_THERMOSTAT_PARAM_TIME_FRIENDLY    "Суточное расписание"
#define CONTROL_THERMOSTAT_PARAM_MODE_KEY         "mode"
#define CONTROL_THERMOSTAT_PARAM_MODE_FRIENDLY    "Режим работы"
#define CONTROL_THERMOSTAT_PARAM_NOTIFY_KEY       "notifications"
#define CONTROL_THERMOSTAT_PARAM_NOTIFY_FRIENDLY  "Уведомления"

#define CONTROL_THERMOSTAT_BOILER_KEY             "boiler"
#define CONTROL_THERMOSTAT_BOILER_TOPIC           "boiler"

#define CONTROL_THERMOSTAT_NOTIFY_KIND            MK_MAIN
#define CONTROL_THERMOSTAT_NOTIFY_PRIORITY        MP_ORDINARY
#define CONTROL_THERMOSTAT_NOTIFY_ALARM           1
#define CONTROL_THERMOSTAT_NOTIFY_ON              "🟠 Работа котла <b>разрешена</b>"
#define CONTROL_THERMOSTAT_NOTIFY_OFF             "🟤 Работа котла <b>заблокирована</b>"

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- Задача --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool sensorsTaskStart();
bool sensorsTaskSuspend();
bool sensorsTaskResume();

#endif // __SENSORS_H__