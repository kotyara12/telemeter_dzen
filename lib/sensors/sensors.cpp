#include "sensors.h"
#include "strings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "project_config.h"
#include "def_consts.h"
#include "esp_timer.h"
#include "rLog.h"
#include "rTypes.h"
#include "rStrings.h"
#include "reStates.h"
#include "reEvents.h"
#include "reMqtt.h"
#include "reEsp32.h"
#include "reI2C.h"
#include "reWiFi.h"
#include "reRangeMonitor.h"
#if CONFIG_TELEGRAM_ENABLE
#include "reTgSend.h"
#endif // CONFIG_TELEGRAM_ENABLE
#if CONFIG_DATASEND_ENABLE
#include "reDataSend.h"
#endif // CONFIG_DATASEND_ENABLE
#if defined(CONFIG_ELTARIFFS_ENABLED) && CONFIG_ELTARIFFS_ENABLED
#include "reElTariffs.h"
#endif // CONFIG_ELTARIFFS_ENABLED

static const char* logTAG = "SENS";
static const char* sensorsTaskName = "sensors";
static TaskHandle_t _sensorsTask;
static bool _sensorsNeedStore = false;

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------- Сенсоры ------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

paramsGroupHandle_t pgSensors;
paramsGroupHandle_t pgIntervals;
paramsGroupHandle_t pgTempMonitor;

static bool sensorsPublish(rSensor *sensor, char* topic, char* payload, const bool free_topic, const bool free_payload)
{
  return mqttPublish(topic, payload, CONFIG_MQTT_SENSORS_QOS, CONFIG_MQTT_SENSORS_RETAINED, free_topic, free_payload);
}

static bool monitorPublish(reRangeMonitor *monitor, char* topic, char* payload, bool free_topic, bool free_payload)
{
  return mqttPublish(topic, payload, CONTROL_TEMP_QOS, CONTROL_TEMP_RETAINED, free_topic, free_payload);
}

static void sensorsMqttTopicsCreate(bool primary)
{
  sensorOutdoor.topicsCreate(primary);
  sensorIndoor.topicsCreate(primary);
  if (tempMonitorIndoor.mqttTopicCreate(primary, CONTROL_TEMP_LOCAL, CONTROL_TEMP_GROUP_TOPIC, CONTROL_TEMP_INDOOR_TOPIC, nullptr)) {
    rlog_i(logTAG, "Generated topic for indoor temperture control: [ %s ]", tempMonitorIndoor.mqttTopicGet());
  };
  sensorBoiler.topicsCreate(primary);
  if (tempMonitorBoiler.mqttTopicCreate(primary, CONTROL_TEMP_LOCAL, CONTROL_TEMP_GROUP_TOPIC, CONTROL_TEMP_BOILER_TOPIC, nullptr)) {
    rlog_i(logTAG, "Generated topic for boiler temperture control: [ %s ]", tempMonitorBoiler.mqttTopicGet());
  };
}

static void sensorsMqttTopicsFree()
{
  sensorOutdoor.topicsFree();
  sensorIndoor.topicsFree();
  tempMonitorIndoor.mqttTopicFree();
  sensorBoiler.topicsFree();
  tempMonitorBoiler.mqttTopicFree();
  rlog_d(logTAG, "Topics for temperture control has been scrapped");
}


static void monitorNotifyIndoor(reRangeMonitor *monitor, range_monitor_status_t status, bool notify, float value, float min, float max)
{
  if (notify) {
    if (status == TMS_NORMAL) {
      tgSend(CONTROL_TEMP_INDOOR_NOTIFY_KIND, CONTROL_TEMP_INDOOR_NOTIFY_PRIORITY, CONTROL_TEMP_INDOOR_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_INDOOR_NOTIFY_NORMAL, value);
    } else if (status == TMS_TOO_LOW) {
      tgSend(CONTROL_TEMP_INDOOR_NOTIFY_KIND, CONTROL_TEMP_INDOOR_NOTIFY_PRIORITY, CONTROL_TEMP_INDOOR_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_INDOOR_NOTIFY_TOO_LOW, value);
    } else if (status == TMS_TOO_HIGH) {
      tgSend(CONTROL_TEMP_INDOOR_NOTIFY_KIND, CONTROL_TEMP_INDOOR_NOTIFY_PRIORITY, CONTROL_TEMP_INDOOR_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_INDOOR_NOTIFY_TOO_HIGH, value);
    } 
  }
}

static void monitorNotifyBoiler(reRangeMonitor *monitor, range_monitor_status_t status, bool notify, float value, float min, float max)
{
  if (notify) {
    if (status == TMS_NORMAL) {
      tgSend(CONTROL_TEMP_BOILER_NOTIFY_KIND, CONTROL_TEMP_BOILER_NOTIFY_PRIORITY, CONTROL_TEMP_BOILER_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_BOILER_NOTIFY_NORMAL, value);
    } else if (status == TMS_TOO_LOW) {
      tgSend(CONTROL_TEMP_BOILER_NOTIFY_KIND, CONTROL_TEMP_BOILER_NOTIFY_PRIORITY, CONTROL_TEMP_BOILER_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_BOILER_NOTIFY_TOO_LOW, value);
    } else if (status == TMS_TOO_HIGH) {
      tgSend(CONTROL_TEMP_BOILER_NOTIFY_KIND, CONTROL_TEMP_BOILER_NOTIFY_PRIORITY, CONTROL_TEMP_BOILER_NOTIFY_ALARM, CONFIG_TELEGRAM_DEVICE, 
        CONTROL_TEMP_BOILER_NOTIFY_TOO_HIGH, value);
    } 
  }
}

static void sensorsStoreData()
{
  rlog_i(logTAG, "Store sensors data");

  sensorOutdoor.nvsStoreExtremums(SENSOR_OUTDOOR_KEY);
  sensorIndoor.nvsStoreExtremums(SENSOR_INDOOR_KEY);
  sensorBoiler.nvsStoreExtremums(SENSOR_BOILER_KEY);

  tempMonitorIndoor.nvsStore(CONTROL_TEMP_INDOOR_KEY);
  tempMonitorBoiler.nvsStore(CONTROL_TEMP_BOILER_KEY);
}

static void sensorsInitParameters()
{
  // Параметры сенсоров и интервалы публикации
  pgSensors = paramsRegisterGroup(nullptr, 
    CONFIG_SENSOR_PGROUP_ROOT_KEY, CONFIG_SENSOR_PGROUP_ROOT_TOPIC, CONFIG_SENSOR_PGROUP_ROOT_FRIENDLY);
  pgIntervals = paramsRegisterGroup(pgSensors, 
    CONFIG_SENSOR_PGROUP_INTERVALS_KEY, CONFIG_SENSOR_PGROUP_INTERVALS_TOPIC, CONFIG_SENSOR_PGROUP_INTERVALS_FRIENDLY);
  pgTempMonitor = paramsRegisterGroup(nullptr, 
    CONTROL_TEMP_GROUP_KEY, CONTROL_TEMP_GROUP_TOPIC, CONTROL_TEMP_GROUP_FRIENDLY);

  // Период публикации данных с сенсоров на MQTT
  paramsRegisterValue(OPT_KIND_PARAMETER, OPT_TYPE_U32, nullptr, pgIntervals,
    CONFIG_SENSOR_PARAM_INTERVAL_MQTT_KEY, CONFIG_SENSOR_PARAM_INTERVAL_MQTT_FRIENDLY,
    CONFIG_MQTT_PARAMS_QOS, (void*)&iMqttPubInterval);

  #if CONFIG_OPENMON_ENABLE
    paramsRegisterValue(OPT_KIND_PARAMETER, OPT_TYPE_U32, nullptr, pgIntervals,
      CONFIG_SENSOR_PARAM_INTERVAL_OPENMON_KEY, CONFIG_SENSOR_PARAM_INTERVAL_OPENMON_FRIENDLY,
      CONFIG_MQTT_PARAMS_QOS, (void*)&iOpenMonInterval);
  #endif // CONFIG_OPENMON_ENABLE

  #if CONFIG_NARODMON_ENABLE
    paramsRegisterValue(OPT_KIND_PARAMETER, OPT_TYPE_U32, nullptr, pgIntervals,
      CONFIG_SENSOR_PARAM_INTERVAL_NARODMON_KEY, CONFIG_SENSOR_PARAM_INTERVAL_NARODMON_FRIENDLY,
      CONFIG_MQTT_PARAMS_QOS, (void*)&iNarodMonInterval);
  #endif // CONFIG_NARODMON_ENABLE

  #if CONFIG_THINGSPEAK_ENABLE
    paramsRegisterValue(OPT_KIND_PARAMETER, OPT_TYPE_U32, nullptr, pgIntervals,
      CONFIG_SENSOR_PARAM_INTERVAL_THINGSPEAK_KEY, CONFIG_SENSOR_PARAM_INTERVAL_THINGSPEAK_FRIENDLY,
      CONFIG_MQTT_PARAMS_QOS, (void*)&iThingSpeakInterval);
  #endif // CONFIG_THINGSPEAK_ENABLE
}

static void sensorsInitSensors()
{
  // Улица
  static rTemperatureItem siOutdoorTemp(nullptr, CONFIG_SENSOR_TEMP_NAME, CONFIG_FORMAT_TEMP_UNIT,
    SENSOR_OUTDOOR_FILTER_MODE, SENSOR_OUTDOOR_FILTER_SIZE, 
    CONFIG_FORMAT_TEMP_VALUE, CONFIG_FORMAT_TEMP_STRING,
    #if CONFIG_SENSOR_TIMESTAMP_ENABLE
      CONFIG_FORMAT_TIMESTAMP_L, 
    #endif // CONFIG_SENSOR_TIMESTAMP_ENABLE
    #if CONFIG_SENSOR_TIMESTRING_ENABLE  
      CONFIG_FORMAT_TIMESTAMP_S, CONFIG_FORMAT_TSVALUE
    #endif // CONFIG_SENSOR_TIMESTRING_ENABLE
  );
  static rSensorItem siOutdoorHum(nullptr, CONFIG_SENSOR_HUMIDITY_NAME, 
    SENSOR_OUTDOOR_FILTER_MODE, SENSOR_OUTDOOR_FILTER_SIZE, 
    CONFIG_FORMAT_HUMIDITY_VALUE, CONFIG_FORMAT_HUMIDITY_STRING,
    #if CONFIG_SENSOR_TIMESTAMP_ENABLE
      CONFIG_FORMAT_TIMESTAMP_L, 
    #endif // CONFIG_SENSOR_TIMESTAMP_ENABLE
    #if CONFIG_SENSOR_TIMESTRING_ENABLE  
      CONFIG_FORMAT_TIMESTAMP_S, CONFIG_FORMAT_TSVALUE
    #endif // CONFIG_SENSOR_TIMESTRING_ENABLE
  );
  sensorOutdoor.initExtItems(SENSOR_OUTDOOR_NAME, SENSOR_OUTDOOR_TOPIC, false,
    DHT_DHT22, CONFIG_GPIO_AM2320, false, CONFIG_GPIO_RELAY_AM2320, 1,
    &siOutdoorHum, &siOutdoorTemp,
    3000, SENSOR_OUTDOOR_ERRORS_LIMIT, nullptr, sensorsPublish);
  sensorOutdoor.registerParameters(pgSensors, SENSOR_OUTDOOR_KEY, SENSOR_OUTDOOR_TOPIC, SENSOR_OUTDOOR_NAME);
  sensorOutdoor.nvsRestoreExtremums(SENSOR_OUTDOOR_KEY);

  // Комната
  static rTemperatureItem siIndoorTemp(nullptr, CONFIG_SENSOR_TEMP_NAME, CONFIG_FORMAT_TEMP_UNIT,
    SENSOR_INDOOR_FILTER_MODE, SENSOR_INDOOR_FILTER_SIZE, 
    CONFIG_FORMAT_TEMP_VALUE, CONFIG_FORMAT_TEMP_STRING,
    #if CONFIG_SENSOR_TIMESTAMP_ENABLE
      CONFIG_FORMAT_TIMESTAMP_L, 
    #endif // CONFIG_SENSOR_TIMESTAMP_ENABLE
    #if CONFIG_SENSOR_TIMESTRING_ENABLE  
      CONFIG_FORMAT_TIMESTAMP_S, CONFIG_FORMAT_TSVALUE
    #endif // CONFIG_SENSOR_TIMESTRING_ENABLE
  );
  static rSensorItem siIndoorHum(nullptr, CONFIG_SENSOR_HUMIDITY_NAME, 
    SENSOR_INDOOR_FILTER_MODE, SENSOR_INDOOR_FILTER_SIZE, 
    CONFIG_FORMAT_HUMIDITY_VALUE, CONFIG_FORMAT_HUMIDITY_STRING,
    #if CONFIG_SENSOR_TIMESTAMP_ENABLE
      CONFIG_FORMAT_TIMESTAMP_L, 
    #endif // CONFIG_SENSOR_TIMESTAMP_ENABLE
    #if CONFIG_SENSOR_TIMESTRING_ENABLE  
      CONFIG_FORMAT_TIMESTAMP_S, CONFIG_FORMAT_TSVALUE
    #endif // CONFIG_SENSOR_TIMESTRING_ENABLE
  );
  sensorIndoor.initExtItems(SENSOR_INDOOR_NAME, SENSOR_INDOOR_TOPIC, false,
    SENSOR_INDOOR_BUS, HTU2X_RES_RH12_TEMP14, false,
    &siIndoorHum, &siIndoorTemp,
    3000, SENSOR_INDOOR_ERRORS_LIMIT, nullptr, sensorsPublish);
  sensorIndoor.registerParameters(pgSensors, SENSOR_INDOOR_KEY, SENSOR_INDOOR_TOPIC, SENSOR_INDOOR_NAME);
  sensorIndoor.nvsRestoreExtremums(SENSOR_INDOOR_KEY);
  tempMonitorIndoor.nvsRestore(CONTROL_TEMP_INDOOR_KEY);
  tempMonitorIndoor.setStatusCallback(monitorNotifyIndoor);
  tempMonitorIndoor.mqttSetCallback(monitorPublish);
  tempMonitorIndoor.paramsRegister(pgTempMonitor, CONTROL_TEMP_INDOOR_KEY, CONTROL_TEMP_INDOOR_TOPIC, CONTROL_TEMP_INDOOR_FRIENDLY);

  // Теплоноситель
  static rTemperatureItem siBoilerTemp(nullptr, CONFIG_SENSOR_TEMP_NAME, CONFIG_FORMAT_TEMP_UNIT,
    SENSOR_BOILER_FILTER_MODE, SENSOR_BOILER_FILTER_SIZE, 
    CONFIG_FORMAT_TEMP_VALUE, CONFIG_FORMAT_TEMP_STRING,
    #if CONFIG_SENSOR_TIMESTAMP_ENABLE
      CONFIG_FORMAT_TIMESTAMP_L, 
    #endif // CONFIG_SENSOR_TIMESTAMP_ENABLE
    #if CONFIG_SENSOR_TIMESTRING_ENABLE  
      CONFIG_FORMAT_TIMESTAMP_S, CONFIG_FORMAT_TSVALUE
    #endif // CONFIG_SENSOR_TIMESTRING_ENABLE
  );
  sensorBoiler.initExtItems(SENSOR_BOILER_NAME, SENSOR_BOILER_TOPIC, false,
    (gpio_num_t)CONFIG_GPIO_DS18B20, ONEWIRE_NONE, 1, DS18x20_RESOLUTION_12_BIT, true, 
    &siBoilerTemp,
    3000, SENSOR_BOILER_ERRORS_LIMIT, nullptr, sensorsPublish);
  sensorBoiler.registerParameters(pgSensors, SENSOR_BOILER_KEY, SENSOR_BOILER_TOPIC, SENSOR_BOILER_NAME);
  sensorBoiler.nvsRestoreExtremums(SENSOR_BOILER_KEY);
  tempMonitorBoiler.nvsRestore(CONTROL_TEMP_BOILER_KEY);
  tempMonitorBoiler.setStatusCallback(monitorNotifyBoiler);
  tempMonitorBoiler.mqttSetCallback(monitorPublish);
  tempMonitorBoiler.paramsRegister(pgTempMonitor, CONTROL_TEMP_BOILER_KEY, CONTROL_TEMP_BOILER_TOPIC, CONTROL_TEMP_BOILER_FRIENDLY);

  espRegisterShutdownHandler(sensorsStoreData); // #2
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------- MQTT ---------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

static void sensorsMqttEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  // MQTT connected
  if (event_id == RE_MQTT_CONNECTED) {
    re_mqtt_event_data_t* data = (re_mqtt_event_data_t*)event_data;
    sensorsMqttTopicsCreate(data->primary);
  } 
  // MQTT disconnected
  else if ((event_id == RE_MQTT_CONN_LOST) || (event_id == RE_MQTT_CONN_FAILED)) {
    sensorsMqttTopicsFree();
  }
}

static void sensorsTimeEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_id == RE_TIME_START_OF_DAY) {
    _sensorsNeedStore = true;
  };
}

static void sensorsResetExtremumsSensor(rSensor* sensor, const char* sensor_name, uint8_t mode) 
{ 
  if (mode == 0) {
    sensor->resetExtremumsTotal();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_TOTAL_DEV, sensor_name);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 1) {
    sensor->resetExtremumsDaily();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_DAILY_DEV, sensor_name);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 2) {
    sensor->resetExtremumsWeekly();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_WEEKLY_DEV, sensor_name);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 3) {
    sensor->resetExtremumsEntirely();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_ENTIRELY_DEV, sensor_name);
    #endif // CONFIG_TELEGRAM_ENABLE
  };
}

static void sensorsResetExtremumsSensors(uint8_t mode)
{
  if (mode == 0) {
    sensorOutdoor.resetExtremumsTotal();
    sensorIndoor.resetExtremumsTotal();
    sensorBoiler.resetExtremumsTotal();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_TOTAL_ALL);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 1) {
    sensorOutdoor.resetExtremumsDaily();
    sensorIndoor.resetExtremumsDaily();
    sensorBoiler.resetExtremumsDaily();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_DAILY_ALL);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 2) {
    sensorOutdoor.resetExtremumsWeekly();
    sensorIndoor.resetExtremumsWeekly();
    sensorBoiler.resetExtremumsWeekly();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_WEEKLY_ALL);
    #endif // CONFIG_TELEGRAM_ENABLE
  } else if (mode == 3) {
    sensorOutdoor.resetExtremumsEntirely();
    sensorIndoor.resetExtremumsEntirely();
    sensorBoiler.resetExtremumsEntirely();
    #if CONFIG_TELEGRAM_ENABLE
      tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
        CONFIG_MESSAGE_TG_SENSOR_CLREXTR_ENTIRELY_ALL);
    #endif // CONFIG_TELEGRAM_ENABLE
  };
};

static void sensorsCommandsEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if ((event_id == RE_SYS_COMMAND) && (event_data)) {
    char* buf = malloc_string((char*)event_data);
    if (buf != nullptr) {
      const char* seps = " ";
      char* cmd = nullptr;
      char* mode = nullptr;
      char* sensor = nullptr;
      uint8_t imode = 0;
      cmd = strtok(buf, seps);
      if ((cmd != nullptr) && (strcasecmp(cmd, CONFIG_SENSOR_COMMAND_EXTR_RESET) == 0)) {
        rlog_i(logTAG, "Reset extremums: %s", buf);
        sensor = strtok(nullptr, seps);
        if (sensor != nullptr) {
          mode = strtok(nullptr, seps);
        };
      
        // Опрделение режима сброса
        if (mode == nullptr) {
          // Возможно, вторым токеном идет режим, в этом случае сбрасываем для всех сенсоров
          if (sensor) {
            if (strcasecmp(sensor, CONFIG_SENSOR_EXTREMUMS_DAILY) == 0) {
              sensor = nullptr;
              imode = 1;
            } else if (strcasecmp(sensor, CONFIG_SENSOR_EXTREMUMS_WEEKLY) == 0) {
              sensor = nullptr;
              imode = 2;
            } else if (strcasecmp(sensor, CONFIG_SENSOR_EXTREMUMS_ENTIRELY) == 0) {
              sensor = nullptr;
              imode = 3;
            };
          };
        } else if (strcasecmp(mode, CONFIG_SENSOR_EXTREMUMS_DAILY) == 0) {
          imode = 1;
        } else if (strcasecmp(mode, CONFIG_SENSOR_EXTREMUMS_WEEKLY) == 0) {
          imode = 2;
        } else if (strcasecmp(mode, CONFIG_SENSOR_EXTREMUMS_ENTIRELY) == 0) {
          imode = 3;
        };

        // Определение сенсора
        if ((sensor == nullptr) || (strcasecmp(sensor, CONFIG_SENSOR_COMMAND_SENSORS_PREFIX) == 0)) {
          sensorsResetExtremumsSensors(imode);
        } else {
          if (strcasecmp(sensor, SENSOR_OUTDOOR_TOPIC) == 0) {
            sensorsResetExtremumsSensor(&sensorOutdoor, SENSOR_OUTDOOR_TOPIC, imode);
          } else if (strcasecmp(sensor, SENSOR_INDOOR_TOPIC) == 0) {
            sensorsResetExtremumsSensor(&sensorIndoor, SENSOR_INDOOR_TOPIC, imode);
          } else if (strcasecmp(sensor, SENSOR_BOILER_TOPIC) == 0) {
            sensorsResetExtremumsSensor(&sensorBoiler, SENSOR_BOILER_TOPIC, imode);
          } else {
            rlog_w(logTAG, "Sensor [ %s ] not found", sensor);
            #if CONFIG_TELEGRAM_ENABLE
              tgSend(CONFIG_SENSOR_COMMAND_KIND, CONFIG_SENSOR_COMMAND_PRIORITY, CONFIG_SENSOR_COMMAND_NOTIFY, CONFIG_TELEGRAM_DEVICE,
                CONFIG_MESSAGE_TG_SENSOR_CLREXTR_UNKNOWN, sensor);
            #endif // CONFIG_TELEGRAM_ENABLE
          };
        };
      };
    };
    if (buf != nullptr) free(buf);
  };
}

static void sensorsOtaEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if ((event_id == RE_SYS_OTA) && (event_data)) {
    re_system_event_data_t* data = (re_system_event_data_t*)event_data;
    if (data->type == RE_SYS_SET) {
      sensorsTaskSuspend();
    } else {
      sensorsTaskResume();
    };
  };
}

bool sensorsEventHandlersRegister()
{
  return eventHandlerRegister(RE_MQTT_EVENTS, ESP_EVENT_ANY_ID, &sensorsMqttEventHandler, nullptr) 
      && eventHandlerRegister(RE_TIME_EVENTS, RE_TIME_START_OF_DAY, &sensorsTimeEventHandler, nullptr)
      && eventHandlerRegister(RE_SYSTEM_EVENTS, RE_SYS_COMMAND, &sensorsCommandsEventHandler, nullptr)
      && eventHandlerRegister(RE_SYSTEM_EVENTS, RE_SYS_OTA, &sensorsOtaEventHandler, nullptr);
}

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- Задача --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void sensorsTaskExec(void *pvParameters)
{
  static TickType_t prevTicks = xTaskGetTickCount();

  // -------------------------------------------------------------------------------------------------------
  // Инициализация параметров
  // -------------------------------------------------------------------------------------------------------
  sensorsInitParameters();

  // -------------------------------------------------------------------------------------------------------
  // Инициализация сенсоров 
  // -------------------------------------------------------------------------------------------------------
  sensorsInitSensors();

  // -------------------------------------------------------------------------------------------------------
  // Инициализация контроллеров
  // -------------------------------------------------------------------------------------------------------
  // Инициализация контроллеров OpenMon
  #if CONFIG_OPENMON_ENABLE
    dsChannelInit(EDS_OPENMON, 
      CONFIG_OPENMON_CTR01_ID, CONFIG_OPENMON_CTR01_TOKEN, 
      CONFIG_OPENMON_MIN_INTERVAL, CONFIG_OPENMON_ERROR_INTERVAL);
  #endif // CONFIG_OPENMON_ENABLE
  
  // Инициализация контроллеров NarodMon
  #if CONFIG_NARODMON_ENABLE
    dsChannelInit(EDS_NARODMON, 
      CONFIG_NARODMON_DEVICE01_ID, CONFIG_NARODMON_DEVICE01_KEY, 
      CONFIG_NARODMON_MIN_INTERVAL, CONFIG_NARODMON_ERROR_INTERVAL);
  #endif // CONFIG_NARODMON_ENABLE

  // Инициализация каналов ThingSpeak
  #if CONFIG_THINGSPEAK_ENABLE
    dsChannelInit(EDS_THINGSPEAK, 
      CONFIG_THINGSPEAK_CHANNEL01_ID, CONFIG_THINGSPEAK_CHANNEL01_KEY, 
      CONFIG_THINGSPEAK_MIN_INTERVAL, CONFIG_THINGSPEAK_ERROR_INTERVAL);
  #endif // CONFIG_THINGSPEAK_ENABLE

  // -------------------------------------------------------------------------------------------------------
  // Таймеры публикции данных с сенсоров
  // -------------------------------------------------------------------------------------------------------
  esp_timer_t mqttPubTimer;
  timerSet(&mqttPubTimer, iMqttPubInterval*1000);
  #if CONFIG_OPENMON_ENABLE
    esp_timer_t omSendTimer;
    timerSet(&omSendTimer, iOpenMonInterval*1000);
  #endif // CONFIG_OPENMON_ENABLE
  #if CONFIG_NARODMON_ENABLE
    esp_timer_t nmSendTimer;
    timerSet(&nmSendTimer, iNarodMonInterval*1000);
  #endif // CONFIG_NARODMON_ENABLE
  #if CONFIG_THINGSPEAK_ENABLE
    esp_timer_t tsSendTimer;
    timerSet(&tsSendTimer, iThingSpeakInterval*1000);
  #endif // CONFIG_THINGSPEAK_ENABLE

  while (1) {
    // -----------------------------------------------------------------------------------------------------
    // Чтение данных с сенсоров
    // -----------------------------------------------------------------------------------------------------
    sensorOutdoor.readData();
    if (sensorOutdoor.getStatus() == SENSOR_STATUS_OK) {
      rlog_i("OUTDOOR", "Values raw: %.2f °С / %.2f %% | out: %.2f °С / %.2f %% | min: %.2f °С / %.2f %% | max: %.2f °С / %.2f %%", 
        sensorOutdoor.getValue2(false).rawValue, sensorOutdoor.getValue1(false).rawValue, 
        sensorOutdoor.getValue2(false).filteredValue, sensorOutdoor.getValue1(false).filteredValue, 
        sensorOutdoor.getExtremumsDaily2(false).minValue.filteredValue, sensorOutdoor.getExtremumsDaily1(false).minValue.filteredValue, 
        sensorOutdoor.getExtremumsDaily2(false).maxValue.filteredValue, sensorOutdoor.getExtremumsDaily1(false).maxValue.filteredValue);
    };
    sensorIndoor.readData();
    if (sensorIndoor.getStatus() == SENSOR_STATUS_OK) {
      rlog_i("INDOOR", "Values raw: %.2f °С / %.2f %% | out: %.2f °С / %.2f %% | min: %.2f °С / %.2f %% | max: %.2f °С / %.2f %%", 
        sensorIndoor.getValue2(false).rawValue, sensorIndoor.getValue1(false).rawValue, 
        sensorIndoor.getValue2(false).filteredValue, sensorIndoor.getValue1(false).filteredValue, 
        sensorIndoor.getExtremumsDaily2(false).minValue.filteredValue, sensorIndoor.getExtremumsDaily1(false).minValue.filteredValue, 
        sensorIndoor.getExtremumsDaily2(false).maxValue.filteredValue, sensorIndoor.getExtremumsDaily1(false).maxValue.filteredValue);
    };
    sensorBoiler.readData();
    if (sensorBoiler.getStatus() == SENSOR_STATUS_OK) {
      rlog_i("BOILER", "Values raw: %.2f °С | out: %.2f °С | min: %.2f °С | max: %.2f °С", 
        sensorBoiler.getValue(false).rawValue, 
        sensorBoiler.getValue(false).filteredValue,
        sensorBoiler.getExtremumsDaily(false).minValue.filteredValue,
        sensorBoiler.getExtremumsDaily(false).maxValue.filteredValue);
    };

    // -----------------------------------------------------------------------------------------------------
    // Контроль температуры
    // -----------------------------------------------------------------------------------------------------

    if (sensorIndoor.getStatus() == SENSOR_STATUS_OK) {
      tempMonitorIndoor.checkValue(sensorIndoor.getValue2(false).filteredValue);
    };
    if (sensorBoiler.getStatus() == SENSOR_STATUS_OK) {
      tempMonitorBoiler.checkValue(sensorBoiler.getValue(false).filteredValue);
    };

    // -----------------------------------------------------------------------------------------------------
    // Сохранение данных сенсоров
    // -----------------------------------------------------------------------------------------------------

    if (_sensorsNeedStore) {
      _sensorsNeedStore = false;
      sensorsStoreData();
    };

    // -----------------------------------------------------------------------------------------------------
    // Публикация данных с сенсоров
    // -----------------------------------------------------------------------------------------------------

    // MQTT брокер
    if (esp_heap_free_check() && statesMqttIsConnected() && timerTimeout(&mqttPubTimer)) {
      timerSet(&mqttPubTimer, iMqttPubInterval*1000);
      sensorOutdoor.publishData(false);
      sensorIndoor.publishData(false);
      tempMonitorIndoor.mqttPublish();
      sensorBoiler.publishData(false);
      tempMonitorBoiler.mqttPublish();
    };

    // open-monitoring.online
    #if CONFIG_OPENMON_ENABLE
      if (statesInetIsAvailabled() && timerTimeout(&omSendTimer)) {
        timerSet(&omSendTimer, iOpenMonInterval*1000);
        char * omValues = nullptr;
        // 01,02: Улица температура (RAW+FLT): FLOAT
        // 03,04: Улица влажность (RAW+FLT): FLOAT
        if (sensorOutdoor.getStatus() == SENSOR_STATUS_OK) {
          omValues = concat_strings_div(omValues, 
            malloc_stringf("p1=%.3f&p2=%.2f&p3=%.3f&p4=%.2f", 
              sensorOutdoor.getValue2(false).rawValue, sensorOutdoor.getValue2(false).filteredValue,
              sensorOutdoor.getValue1(false).rawValue, sensorOutdoor.getValue1(false).filteredValue),
            "&");
        };
        // 05,06: Комната температура (RAW+FLT): FLOAT
        // 07,08: Комната влажность (RAW+FLT): FLOAT
        if (sensorIndoor.getStatus() == SENSOR_STATUS_OK) {
          omValues = concat_strings_div(omValues, 
            malloc_stringf("p5=%.3f&p6=%.2f&p7=%.3f&p8=%.2f", 
              sensorIndoor.getValue2(false).rawValue, sensorIndoor.getValue2(false).filteredValue,
              sensorIndoor.getValue1(false).rawValue, sensorIndoor.getValue1(false).filteredValue),
            "&");
        };
        // 09,10: Теплоноситель (RAW+FLT): FLOAT
        if (sensorBoiler.getStatus() == SENSOR_STATUS_OK) {
          omValues = concat_strings_div(omValues, 
            malloc_stringf("p9=%.3f&p10=%.2f", 
              sensorBoiler.getValue(false).rawValue, sensorBoiler.getValue(false).filteredValue),
            "&");
        };
        // Отправляем данные
        if (omValues) {
          dsSend(EDS_OPENMON, CONFIG_OPENMON_CTR01_ID, omValues, false); 
          free(omValues);
        };
      };
    #endif // CONFIG_OPENMON_ENABLE

    // narodmon.ru
    #if CONFIG_NARODMON_ENABLE
      if (statesInetIsAvailabled() && timerTimeout(&nmSendTimer) && (sensorOutdoor.getStatus() == SENSOR_STATUS_OK)) {
        timerSet(&nmSendTimer, iNarodMonInterval*1000);
        dsSend(EDS_NARODMON, CONFIG_NARODMON_DEVICE01_ID, malloc_stringf("T1=%.2f&H1=%.2f",
          sensorOutdoor.getValue2(false).filteredValue, sensorOutdoor.getValue1(false).filteredValue));
      };
    #endif // CONFIG_NARODMON_ENABLE

    // thingspeak.com
    #if CONFIG_THINGSPEAK_ENABLE
      if (statesInetIsAvailabled() && timerTimeout(&tsSendTimer)) {
        timerSet(&tsSendTimer, iThingSpeakInterval*1000);
      };
    #endif // CONFIG_THINGSPEAK_ENABLE
    // -----------------------------------------------------------------------------------------------------
    // Ожидание
    // -----------------------------------------------------------------------------------------------------
    vTaskDelayUntil(&prevTicks, pdMS_TO_TICKS(CONFIG_SENSORS_TASK_CYCLE));
  };

  vTaskDelete(nullptr);
  espRestart(RR_UNKNOWN);
}

bool sensorsTaskStart()
{
  #if CONFIG_SENSORS_STATIC_ALLOCATION
    static StaticTask_t sensorsTaskBuffer;
    static StackType_t sensorsTaskStack[CONFIG_SENSORS_TASK_STACK_SIZE];
    _sensorsTask = xTaskCreateStaticPinnedToCore(sensorsTaskExec, sensorsTaskName, 
      CONFIG_SENSORS_TASK_STACK_SIZE, NULL, CONFIG_TASK_PRIORITY_SENSORS, sensorsTaskStack, &sensorsTaskBuffer, CONFIG_TASK_CORE_SENSORS);
  #else
    xTaskCreatePinnedToCore(sensorsTaskExec, sensorsTaskName, 
      CONFIG_SENSORS_TASK_STACK_SIZE, NULL, CONFIG_TASK_PRIORITY_SENSORS, &_sensorsTask, CONFIG_TASK_CORE_SENSORS);
  #endif // CONFIG_SENSORS_STATIC_ALLOCATION
  if (_sensorsTask) {
    rloga_i("Task [ %s ] has been successfully created and started", sensorsTaskName);
    return sensorsEventHandlersRegister();
  }
  else {
    rloga_e("Failed to create a task for processing sensor readings!");
    return false;
  };
}

bool sensorsTaskSuspend()
{
  if ((_sensorsTask) && (eTaskGetState(_sensorsTask) != eSuspended)) {
    vTaskSuspend(_sensorsTask);
    if (eTaskGetState(_sensorsTask) == eSuspended) {
      rloga_d("Task [ %s ] has been suspended", sensorsTaskName);
      return true;
    } else {
      rloga_e("Failed to suspend task [ %s ]!", sensorsTaskName);
    };
  };
  return false;
}

bool sensorsTaskResume()
{
  if ((_sensorsTask) && (eTaskGetState(_sensorsTask) == eSuspended)) {
    vTaskResume(_sensorsTask);
    if (eTaskGetState(_sensorsTask) != eSuspended) {
      rloga_i("Task [ %s ] has been successfully resumed", sensorsTaskName);
      return true;
    } else {
      rloga_e("Failed to resume task [ %s ]!", sensorsTaskName);
    };
  };
  return false;
}


