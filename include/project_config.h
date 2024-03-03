/*
   -----------------------------------------------------------------------------------------------------------------------
   EN: Project configuration file, accessible from all libraries connected to the project
   RU: Файл конфигурации проекта, он должен быть доступен из всех файлов проекта, в том числе и библиотек
   -----------------------------------------------------------------------------------------------------------------------
   (с) 2020-2023 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#pragma once

#include <stdint.h>
#include "esp_task.h"

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------- EN - Version ----------------------------------------------------------
// ----------------------------------------------- RU - Версии -----------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
#define APP_VERSION "20240228.007"
// 20240228.007: Добавлены функции охранно-пожаной-аварийной сигнализации
// 20230625.006: Пример замены сенсоров на другие, обновление библиотек
// 20230219.005: MQTT client: исправлена проблема переключения на резервный MQTT сервер, если он используется
// 20230215.004: Адаптация под версию ESP-IDF 5.0.0
// 20230210.003: Адаптация под ESP-IDF 4.4.3, добавлена поддержка статических буферов для I2C
// 20221205.002: Добавлена отправка данных на Open Monitoring
// 20221010.001: Только телеметрия (без термостата и охранно-пожарной сигнализации)

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------- EN - GPIO -------------------------------------------------------------
// ----------------------------------------------- RU - GPIO -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Peripheral pin number
// RU: Номер выводов периферии, версия платы 9.05
#define CONFIG_GPIO_SYSTEM_LED      33
#define CONFIG_GPIO_ALARM_LED       23
#define CONFIG_GPIO_ALARM_SIREN     25
#define CONFIG_GPIO_ALARM_FLASH     26
// #define CONFIG_GPIO_BUZZER           13
#define CONFIG_GPIO_BUZZER_ACTIVE   13
#define CONFIG_GPIO_RX433           15
#define CONFIG_GPIO_DS18B20         4
#define CONFIG_GPIO_AM2320          0
#define CONFIG_GPIO_RELAY_AM2320    32
#define CONFIG_GPIO_RELAY_BOILER    27
// EN: Alarm zones
// RU: Зоны ОПС
#define CONFIG_GPIO_ALARM_ZONE_1    18
#define CONFIG_GPIO_ALARM_ZONE_2    19
#define CONFIG_GPIO_ALARM_ZONE_3    12
#define CONFIG_GPIO_ALARM_ZONE_4    14
#define CONFIG_GPIO_ALARM_ZONE_5    2
#define CONFIG_GPIO_ALARM_LEVEL     0x01
// EN: I2C bus #0: pins, pullup, frequency
// RU: Шина I2C #0: выводы, подтяжка, частота, размер статического буфера в транзациях
#define CONFIG_I2C_PORT0_SDA        21
#define CONFIG_I2C_PORT0_SCL        22
#define CONFIG_I2C_PORT0_PULLUP     false 
#define CONFIG_I2C_PORT0_FREQ_HZ    100000
#define CONFIG_I2C_PORT0_STATIC     2
// EN: I2C bus #1: pins, pullup, frequency
// RU: Шина I2C #1: выводы, подтяжка, частота, размер статического буфера в транзациях
#define CONFIG_I2C_PORT1_SDA        17
#define CONFIG_I2C_PORT1_SCL        16
#define CONFIG_I2C_PORT1_PULLUP     false
#define CONFIG_I2C_PORT1_FREQ_HZ    100000
#define CONFIG_I2C_PORT1_STATIC     2
// EN: Blocking access to I2C buses. Makes sense if I2C devices are accessed from different threads. 
// The I2C APIs are not thread-safe, if you want to use one I2C port in different tasks, you need to take care of the multi-thread issue.
// RU: Блокировка доступа к шинам I2C. Имеет смысл, если доступ к устройствам I2C осущствляется из разных потоков
// API-интерфейсы I2C не являются потокобезопасными, если вы хотите использовать один порт I2C в разных задачах, вам нужно позаботиться о проблеме многопоточности.
#define CONFIG_I2C_LOCK 0

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------- EN - Common parameters ----------------------------------------------------
// -------------------------------------------- RU - Общие параметры -----------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Restart device on memory allocation errors
// RU: Перезапустить устройство при ошибках выделения памяти
#define CONFIG_HEAP_ALLOC_FAILED_RESTART 0
#define CONFIG_HEAP_ALLOC_FAILED_RETRY 0

/* Silent mode (no sounds, no blinks) */
// EN: Allow "quiet" mode. Quiet mode is the period of time of day when LED flashes and sounds are blocked.
// RU: Разрешить "тихий" режим. Тихий режим - это период времени суток, когда блокируются вспышки светодиодов и звуки.
#define CONFIG_SILENT_MODE_ENABLE 1
#define CONFIG_SILENT_MODE_EXTENDED 0
// EN: Interval in H1M1H2M2 format. That is, the interval 21:00 -> 06:00 is 21000600
// RU: Интервал в формате H1M1H2M2. То есть интервал 21:00 -> 06:00 это 21000600
#define CONFIG_SILENT_MODE_INTERVAL 22000600UL

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------- EN - Temperature control ------------------------------------------------
// --------------------------------------------- RU - Контроль температуры -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Here you can specify any parameters related to the main task of the device
// RU: Здесь можно указать вообще любые параметры, связанные с прикладной задачей устройства

// EN: Interval of reading data from sensors in milliseconds
// RU: Интервал чтения данных с сенсоров в миллисекундах
#define CONFIG_SENSORS_TASK_CYCLE 30000
// EN: Use static memory allocation for the task and queue. CONFIG_FREERTOS_SUPPORT_STATIC_ALLOCATION must be enabled!
// RU: Использовать статическое выделение памяти под задачу и очередь. Должен быть включен параметр CONFIG_FREERTOS_SUPPORT_STATIC_ALLOCATION!
#define CONFIG_SENSORS_STATIC_ALLOCATION 1
// EN: Stack size for main task
// RU: Размер стека для главной задачи
#define CONFIG_SENSORS_TASK_STACK_SIZE 4*1024

// EN: Allow publishing of raw RAW data (no correction or filtering): 0 - only processed value, 1 - always both values, 2 - only when there is processing
// RU: Разрешить публикацию необработанных RAW-данных (без коррекции и фильтрации): 0 - только обработанное значение, 1 - всегда оба значения, 2 - только когда есть обработка
#define CONFIG_SENSOR_RAW_ENABLE 1
// EN: Allow publication of sensor status
// RU: Разрешить публикацию форматированных данных в виде строки
#define CONFIG_SENSOR_STRING_ENABLE 0
// EN: Allow the publication of the time stamp of reading data from the sensor
// RU: Разрешить публикацию отметки времени чтения данных с сенсора
#define CONFIG_SENSOR_TIMESTAMP_ENABLE 1
// EN: Allow the publication of formatted data as "value + time"
// RU: Разрешить публикацию форматированных данных в виде "значение + время"
#define CONFIG_SENSOR_TIMESTRING_ENABLE 1
// EN: Allow dew point calculation and publication
// RU: Разрешить вычисление и публикацию точки росы
#define CONFIG_SENSOR_DEWPOINT_ENABLE 0
// EN: Allow publishing of mixed value, for example "temperature + humidity"
// RU: Разрешить публикацию смешанного значения, например "температура + влажность"
#define CONFIG_SENSOR_DISPLAY_ENABLED 1
// EN: Allow publication of absolute minimum and maximum
// RU: Разрешить публикацию абсолютного минимума и максимума
#define CONFIG_SENSOR_EXTREMUMS_ENTIRELY_ENABLE 1
// EN: Allow publication of daily minimum and maximum
// RU: Разрешить публикацию ежедневного минимума и максимума
#define CONFIG_SENSOR_EXTREMUMS_DAILY_ENABLE 1
// EN: Allow publication of weekly minimum and maximum
// RU: Разрешить публикацию еженедельного минимума и максимума
#define CONFIG_SENSOR_EXTREMUMS_WEEKLY_ENABLE 1
// EN: Publish extremums only when they are changed
// RU: Публиковать экстеремумы только при их изменении
#define CONFIG_SENSOR_EXTREMUMS_OPTIMIZED 1

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- EN - Security ------------------------------------------------------
// ------------------------------------------------ RU - Сигнализация ----------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// EN: Use static memory allocation for the fire alarm task
// RU: Использовать статическое выделение памяти для задачи охранно-пожарной сигнализации
#define CONFIG_ALARM_STATIC_ALLOCATION 1
// EN: Stack size for the fire alarm task
// RU: Размер стека для задачи охранно-пожарной сигнализации
#define CONFIG_ALARM_STACK_SIZE 4098
// EN: Queue size for the fire alarm task
// RU: Размер очереди для задачи охранно-пожарной сигнализации
#define CONFIG_ALARM_QUEUE_SIZE 32
// EN: Device topic for OPS
// RU: Топик устройства для ОПС
// #define CONFIG_ALARM_MQTT_DEVICE_TOPIC "home"
// EN: Publish the status of OPS sensors in local topics for transmission to other devices
// RU: Публиковать состояние сенсоров ОПС в локальных топиках для передачи на другие устройства
#define CONFIG_ALARM_LOCAL_PUBLISH true
// EN: Scheme of OPS topics: 0 - %location%/config/security/mode; 1 - %location%/%device%/config/security/mode
// RU: Схема топиков ОПС: 0 - %location%/config/security/mode; 1 - %location%/%device%/config/security/mode
#define CONFIG_ALARM_MQTT_DEVICE_MODE 0
// EN: Scheme of OPS topics: 0 - %location%/security/events/%zone%; 1 - %location%/%device%/security/events/%zone%
// RU: Схема топиков ОПС: 0 - %location%/security/events/%zone%; 1 - %location%/%device%/security/events/%zone%
#define CONFIG_ALARM_MQTT_DEVICE_EVENTS 0
// EN: Scheme of OPS topics: 0 - %location%/security/status/%device%; 1 - %location%/%device%/security/status
// RU: Схема топиков ОПС: 0 - %location%/security/status/%device%; 1 - %location%/%device%/security/status
#define CONFIG_ALARM_MQTT_DEVICE_STATUS 0
// EN: When disabling the alarm from the remote control, immediately disarm; otherwise disable the alarm without disarming
// RU: При отключении тревоги с пульта сразу же снять с охраны; иначе отключить тревогу без снятия с охраны
#define CONFIG_ALARM_TOGETHER_DISABLE_SIREN_AND_ALARM 1

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- EN - Wifi networks -----------------------------------------------------
// ------------------------------------------------ RU - WiFi сети -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
/** 
 * EN: Single network mode
 * --------------------
 * Uncomment CONFIG_WIFI_SSID and CONFIG_WIFI_PASS to disable automatic switching between wifi networks
 * 
 * RU: Режим для одной сети
 * --------------------
 * Раскомментируйте CONFIG_WIFI_SSID и CONFIG_WIFI_PASS, чтобы отключить автоматическое переключение между wifi-сетями
 * */
// #define CONFIG_WIFI_SSID "wifi_network_name"
// #define CONFIG_WIFI_PASS "wifi_password"

/** 
 * EN: Multi-network mode
 * --------------------
 * You can define from one to five networks. If it is not possible to connect to one of the networks, ESP will try to connect to the next one.
 * The number of a successful connection is remembered and used later (until the next failure).
 * This allows you to move the device from one building to another without reflashing and reconfiguring it. 
 * Just define all possible networks in advance.
 * 
 * RU: Режим для нескольких сетей
 * --------------------
 * Вы можете определенить от одной до пяти сетей. При невозможности подключиться к одной из сетей, ESP попытается поключиться к следующей. 
 * Номер успешного подключения запоминается и используется в дальнейшем (до следущего сбоя). 
 * Это позволяет переносить устройство из одного здания в другое, не перепрошивая и перенастраивая его. 
 * Просто заранее определите все возможные сети.
 * */
#define CONFIG_WIFI_1_SSID "WIFI1"
#define CONFIG_WIFI_1_PASS "000000000"
#define CONFIG_WIFI_2_SSID "WIFI2"
#define CONFIG_WIFI_2_PASS "111111111"
#define CONFIG_WIFI_3_SSID "WIFI3"
#define CONFIG_WIFI_3_PASS "222222222"
#define CONFIG_WIFI_4_SSID "WIFI4"
#define CONFIG_WIFI_4_PASS "333333333"
#define CONFIG_WIFI_5_SSID "WIFI5"
#define CONFIG_WIFI_5_PASS "444444444"

// EN: WiFi connection parameters. Commenting out these lines will use the default ESP-IDF parameters
// RU: Параметры WiFi подключения. Если закомментировать эти строки, будут использованы параметры по умолчанию ESP-IDF
// #define CONFIG_WIFI_STORAGE   WIFI_STORAGE_RAM
// #define CONFIG_WIFI_BANDWIDTH WIFI_BW_HT20

// EN: Restart the device if there is no WiFi connection for more than the specified time in minutes.
//     Comment out the line if you do not need to restart the device if there is no network connection for a long time
// RU: Перезапустить устройство, если нет подключения к WiFi более указанного времени в минутах. 
//     Закомментируйте строку, если не нужно перезапускать устройство при длительном отсутствии подключения к сети
#define CONFIG_WIFI_TIMER_RESTART_DEVICE 60*24

// EN: Allow periodic check of Internet availability using ping.
//     Sometimes network access may be lost, but WiFi connection works. In this case, the device will suspend all network processes.
// RU: Разрешить периодическую проверку доступности сети интернет с помошью пинга. 
//     Иногда доступ в сеть может пропасть, но подключение к WiFi при этом работает. В этом случае устройство приостановит все сетевые процессы.
#define CONFIG_PINGER_ENABLE 1

// EN: Disable network error indication (wifi, internet, openmon, tg...) as the device is not always connected to the network
// RU: Отключить иникацию сетевых ошибок (wifi, inetnet, openmon, tg...), так как устройство не всегда подключено к сети
#define CONFIG_OFFLINE_MODE 0

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- EN - MQTT client -------------------------------------------------------
// ---------------------------------------------- RU - MQTT клиент -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Parameters of MQTT brokers. Two brokers can be defined: primary and backup
// RU: Параметры MQTT-брокеров. Можно определить два брокера: основной и резервный
// CONFIG_MQTTx_TYPE :: 0 - public, 1 - local, 2 - gateway (CONFIG_MQTT1_HOST not used)

/********************* primary server ************************/
#define CONFIG_MQTT1_TYPE 0
#define CONFIG_MQTT1_HOST "m99.wqtt.ru"
#define CONFIG_MQTT1_PING_CHECK 1
#define CONFIG_MQTT1_PING_CHECK_DURATION 250
#define CONFIG_MQTT1_PING_CHECK_LOSS 50.0
#define CONFIG_MQTT1_PING_CHECK_LIMIT 3
#define CONFIG_MQTT1_PORT_TCP 2632
#define CONFIG_MQTT1_PORT_TLS 2633
#define CONFIG_MQTT1_USERNAME "u_dzen"
#define CONFIG_MQTT1_PASSWORD "aaaaaaaa"
#define CONFIG_MQTT1_TLS_ENABLED 1
#define CONFIG_MQTT1_TLS_STORAGE CONFIG_DEFAULT_TLS_STORAGE
#define CONFIG_MQTT1_TLS_PEM_START CONFIG_DEFAULT_TLS_PEM_START
#define CONFIG_MQTT1_TLS_PEM_END CONFIG_DEFAULT_TLS_PEM_END
#define CONFIG_MQTT1_CLEAN_SESSION 1
#define CONFIG_MQTT1_AUTO_RECONNECT 1
#define CONFIG_MQTT1_KEEP_ALIVE 60
#define CONFIG_MQTT1_TIMEOUT 10000
#define CONFIG_MQTT1_RECONNECT 10000
#define CONFIG_MQTT1_CLIENTID "esp32_dzen1"
// #define CONFIG_MQTT1_LOC_PREFIX ""
// #define CONFIG_MQTT1_PUB_PREFIX ""
#define CONFIG_MQTT1_LOC_LOCATION "dzen/local"
#define CONFIG_MQTT1_PUB_LOCATION "dzen"
#define CONFIG_MQTT1_LOC_DEVICE "thermostat"
#define CONFIG_MQTT1_PUB_DEVICE "thermostat"

/****************** MQTT : pinger ********************/
// EN: Allow the publication of ping results on the MQTT broker
// RU: Разрешить публикацию результатов пинга на MQTT брокере
#define CONFIG_MQTT_PINGER_ENABLE 1
#if CONFIG_MQTT_PINGER_ENABLE
// EN: Ping results topic name
// RU: Название топика результатов пинга
#define CONFIG_MQTT_PINGER_TOPIC "ping"
#define CONFIG_MQTT_PINGER_LOCAL 0
#define CONFIG_MQTT_PINGER_QOS 0
#define CONFIG_MQTT_PINGER_RETAINED 1
#define CONFIG_MQTT_PINGER_AS_PLAIN 0
#define CONFIG_MQTT_PINGER_AS_JSON 1
#endif // CONFIG_MQTT_PINGER_ENABLE

/*************** MQTT : remote control ***************/
// EN: Allow the device to process incoming commands
// RU: Разрешить обработку входящих команд устройством
#define CONFIG_MQTT_COMMAND_ENABLE 1
// EN: Allow OTA updates via a third party server
// RU: Разрешить OTA обновления через сторонний сервер
#define CONFIG_MQTT_OTA_ENABLE 1

/***************** MQTT : sensors ********************/
// EN: Delay between update attempts
// RU: Интервал публикации данных с сенсоров в секундах
#define CONFIG_MQTT_SENSORS_SEND_INTERVAL 600
// EN: QOS for sensor data
// RU: QOS для данных с сенсоров
#define CONFIG_MQTT_SENSORS_QOS 1
#define CONFIG_MQTT_SENSORS_LOCAL_QOS 2
// EN: Save the last sent data on the broker
// RU: Сохранять на брокере последние отправленные данные
#define CONFIG_MQTT_SENSORS_RETAINED 1
#define CONFIG_MQTT_SENSORS_LOCAL_RETAINED 0

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------- EN - http://open-monitoring.online/ --------------------------------------------
// -------------------------------------- RU - http://open-monitoring.online/ --------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Enable sending data to open-monitoring.online
// RU: Включить отправку данных на open-monitoring.online
#define CONFIG_OPENMON_ENABLE 1
#if CONFIG_OPENMON_ENABLE
// EN: Frequency of sending data in seconds
// RU: Периодичность отправки данных в секундах
#define CONFIG_OPENMON_SEND_INTERVAL 300
// EN: Controller ids and tokens for open-monitoring.online
// RU: Идентификаторы контроллеров и токены для open-monitoring.online
#define CONFIG_OPENMON_CTR01_ID 9998
#define CONFIG_OPENMON_CTR01_TOKEN "3W3aYt"
// EN: Allow publication of ping results и системной информации on open-monitoring.online
// RU: Разрешить публикацию результатов пинга и системной информации на open-monitoring.online
#define CONFIG_OPENMON_PINGER_ENABLE 1
#if CONFIG_OPENMON_PINGER_ENABLE
#define CONFIG_OPENMON_PINGER_ID 9999
#define CONFIG_OPENMON_PINGER_TOKEN "LUs2hg"
#define CONFIG_OPENMON_PINGER_INTERVAL 300000
#define CONFIG_OPENMON_PINGER_RSSI 1
#define CONFIG_OPENMON_PINGER_HEAP_FREE 1
#define CONFIG_OPENMON_PINGER_HOSTS 1
#endif // CONFIG_OPENMON_PINGER_ENABLE
#endif // CONFIG_OPENMON_ENABLE

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- EN - https://narodmon.ru/ ----------------------------------------------
// ---------------------------------------------- RU - https://narodmon.ru/ ----------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// EN: Enable sending data to narodmon.ru
// RU: Включить отправку данных на narodmon.ru
#define CONFIG_NARODMON_ENABLE 1
#if CONFIG_NARODMON_ENABLE
// EN: Frequency of sending data in seconds
// RU: Периодичность отправки данных в секундах
#define CONFIG_NARODMON_SEND_INTERVAL 300
#define CONFIG_NARODMON_DEVICE01_ID 81693
#define CONFIG_NARODMON_DEVICE01_KEY "9c:9e:9f:77:18:f7"
#endif // CONFIG_NARODMON_ENABLE

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------- EN - https://thingspeak.com/ ---------------------------------------------
// -------------------------------------------- RU - https://thingspeak.com/ ---------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Enable sending data to thingspeak.com
// RU: Включить отправку данных на thingspeak.com
#define CONFIG_THINGSPEAK_ENABLE 1
#if CONFIG_THINGSPEAK_ENABLE
// EN: Frequency of sending data in seconds
// RU: Периодичность отправки данных в секундах
#define CONFIG_THINGSPEAK_SEND_INTERVAL 300
// EN: Tokens for writing on thingspeak.com
// RU: Токены для записи на thingspeak.com
#define CONFIG_THINGSPEAK_CHANNEL01_ID 99999999
#define CONFIG_THINGSPEAK_CHANNEL01_KEY "LAJ6AA9AKBVTZ7AP"
#endif // CONFIG_THINGSPEAK_ENABLE

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ EN -Sending to your hosting ------------------------------------------------
// ----------------------------------------- RU - Отправка на свой хостинг -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Enable sending data to your hosting
// RU: Включить отправку данных на свой хостинг
#define CONFIG_HOSTING_ENABLE 0

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- EN - Telegram notify ---------------------------------------------------
// ------------------------------------------- RU - Уведомления в Telegram -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Allow Telegram notifications (general flag)
// RU: Разрешить уведомления в Telegram (общий флаг)
#define CONFIG_TELEGRAM_ENABLE 1
// EN: Telegram API bot token
// RU: Токен бота API Telegram
#define CONFIG_TELEGRAM_TOKEN "00000000000:0000000000000000000000000000000000000000000000000000000"
// EN: Chat or group ID
// RU: Идентификатор чата или группы
#define CONFIG_TELEGRAM_CHAT_ID_MAIN     "-00000000000"
#define CONFIG_TELEGRAM_CHAT_ID_SERVICE  "-00000000001"
#define CONFIG_TELEGRAM_CHAT_ID_PARAMS   CONFIG_TELEGRAM_CHAT_ID_SERVICE
#define CONFIG_TELEGRAM_CHAT_ID_SECURITY CONFIG_TELEGRAM_CHAT_ID_MAIN
// EN: Send message header (device name, see CONFIG_TELEGRAM_DEVICE)
// RU: Отправлять заголовок сообщения (название устройства, см. CONFIG_TELEGRAM_DEVICE)
#define CONFIG_TELEGRAM_TITLE_ENABLED 0
// EN: Device name (published as message header)
// RU: Название устройства (публикуется в качестве заголовка сообщения)
#define CONFIG_TELEGRAM_DEVICE "ДЗЕН"

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- EN - Notifies ----------------------------------------------------------
// --------------------------------------------- RU - Уведомления --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Blink the system LED when sending a data
// RU: Мигать системным светодиодом при отправке данных
#define CONFIG_SYSLED_MQTT_ACTIVITY 0
#define CONFIG_SYSLED_SEND_ACTIVITY 0
#define CONFIG_SYSLED_TELEGRAM_ACTIVITY 0
// EN: Allow remote enabling or disabling of notifications without flashing the device
// RU: Разрешить удаленную включение или отключение уведомлений без перепрошивки устройства
#define CONFIG_NOTIFY_TELEGRAM_CUSTOMIZABLE 1
// EN: Telegram notifications when switching the security mode
// RU: Уведомления в Telegram при переключении режима охраны
#define CONFIG_NOTIFY_TELEGRAM_ALARM_MODE_CHANGE 1
#define CONFIG_NOTIFY_TELEGRAM_ALARM_ALERT_MODE_CHANGE 1
// EN: Allow notifications in Telegram if a signal is received from an AFS sensor that is not registered in the system
// RU: Разрешить уведомления в Telegram, если поступил сигнал от датчика ОПС, который не зарегистрирован в системе
#define CONFIG_NOTIFY_TELEGRAM_ALARM_SENSOR_UNDEFINED 0
#define CONFIG_NOTIFY_TELEGRAM_ALARM_ALERT_SENSOR_UNDEFINED 0
// EN: Allow notifications in Telegram if an unregistered command has been received from the AFS sensor
// RU: Разрешить уведомления в Telegram, если поступила незарегистрованная команда от датчика ОПС
#define CONFIG_NOTIFY_TELEGRAM_ALARM_COMMAND_UNDEFINED 0
#define CONFIG_NOTIFY_TELEGRAM_ALARM_ALERT_COMMAND_UNDEFINED 0
// EN: Telegram notifications for security alarms
// RU: Уведомления в Telegram при тревогах охраны
#define CONFIG_NOTIFY_TELEGRAM_ALARM_ALARM 1
#define CONFIG_NOTIFY_TELEGRAM_ALARM_ALERT_ALARM 1

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- EN - Sensors -------------------------------------------------------
// -------------------------------------------------- RU - Сенсоры -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Allow the publication of sensor data in a simple form (each value in a separate subtopic)
// RU: Разрешить публикацию данных сенсора в простом виде (каждое значение в отдельном субтопике)
#define CONFIG_SENSOR_AS_PLAIN 0
// EN: Allow the publication of sensor data as JSON in one topic
// RU: Разрешить публикацию данных сенсора в виде JSON в одном топике
#define CONFIG_SENSOR_AS_JSON 1
// EN: Allow publication of sensor status
// RU: Разрешить публикацию статуса сенсора
#define CONFIG_SENSOR_STATUS_ENABLE 1

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ EN - Electricity tariffs ---------------------------------------------------
// ----------------------------------------- RU - Тарифы электроэнергии --------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// EN: Use switching between multiple electricity tariffs
// RU: Использовать переключение между несколькими тарифами электроэнергии
#define CONFIG_ELTARIFFS_ENABLED 0
// EN: Maximum number of tariffs
// RU: Максимальное количество тарифов
#define CONFIG_ELTARIFFS_COUNT 2
// EN: Type of values for tariff parameters (OPT_KIND_PARAMETER - only for this device, OPT_KIND_PARAMETER_LOCATION - for all location devices)
// RU: Тип занчений для параметров тарифов (OPT_KIND_PARAMETER - только для этого устройства, OPT_KIND_PARAMETER_LOCATION - для всех устройств локации)
#define CONFIG_ELTARIFFS_PARAMS_TYPE OPT_KIND_PARAMETER_LOCATION
// EN: Tariff parameter values: days of the week, time intervals and cost of 1 kW/h
// RU: Значения параметров тарифов: дни недели, интервалы времени и стоимость 1кВт/ч
#define CONFIG_ELTARIFFS_TARIF1_DAYS WEEK_ANY
#define CONFIG_ELTARIFFS_TARIF1_TIMESPAN 7002300UL
#define CONFIG_ELTARIFFS_TARIF1_PRICE 8.02
#define CONFIG_ELTARIFFS_TARIF2_DAYS WEEK_ANY
#define CONFIG_ELTARIFFS_TARIF2_TIMESPAN 23000700UL
#define CONFIG_ELTARIFFS_TARIF2_PRICE 4.18

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- EN - Log -----------------------------------------------------------
// ------------------------------------------------ RU - Отладка ---------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// EN: Debug message level. Anything above this level will be excluded from the code
// RU: Уровень отладочных сообщений. Всё, что выше этого уровня, будет исключено из кода
#define CONFIG_RLOG_PROJECT_LEVEL RLOG_LEVEL_DEBUG
// EN: Add text color codes to messages. Doesn't work on Win7
// RU: Добавлять коды цвета текста к сообщениям. Не работает на Win7
#define CONFIG_RLOG_PROJECT_COLORS 1
// EN: Add time stamp to messages
// RU: Добавлять отметку времени к сообщениям
#define CONFIG_RLOG_SHOW_TIMESTAMP 1
// EN: Add file and line information to messages
// RU: Добавлять информацию о файле и строке к сообщениям
#define CONFIG_RLOG_SHOW_FILEINFO 0


// EN: Preserve debugging information across device software restarts
// RU: Сохранять отладочную информацию при программном перезапуске устройства
#define CONFIG_RESTART_DEBUG_INFO 1
// EN: Allow heap information to be saved periodically, with subsequent output on restart
// RU: Разрешить периодическое сохранение информации о куче / памяти с последующем выводом при перезапуске
#define CONFIG_RESTART_DEBUG_HEAP_SIZE_SCHEDULE 1
// EN: Depth to save the processor stack on restart. 0 - do not save
// RU: Глубина сохранения стека процессора при перезапуске. 0 - не сохранять
#define CONFIG_RESTART_DEBUG_STACK_DEPTH 28
// EN: Allow publishing debug info from WiFi module
// RU: Разрешить публикацию отладочной информации из модуля WiFi
#define CONFIG_WIFI_DEBUG_ENABLE 0
// EN: Allow periodic publication of system information
// RU: Разрешить периодическую публикацию системной информации
#define CONFIG_MQTT_SYSINFO_ENABLE 1
#define CONFIG_MQTT_SYSINFO_INTERVAL 1000*60*5
#define CONFIG_MQTT_SYSINFO_SYSTEM_FLAGS 1           
// EN: Allow periodic publication of task information. CONFIG_FREERTOS_USE_TRACE_FACILITY / configUSE_TRACE_FACILITY must be allowed
// RU: Разрешить периодическую публикацию информации о задачах. Должен быть разрешен CONFIG_FREERTOS_USE_TRACE_FACILITY / configUSE_TRACE_FACILITY
#define CONFIG_MQTT_TASKLIST_ENABLE 0
#define CONFIG_MQTT_TASKLIST_INTERVAL 1000*60*60

