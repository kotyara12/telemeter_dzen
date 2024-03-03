#include "security.h"
#include <stdbool.h>
#include "project_config.h"
#include "def_alarm.h"
#include "def_consts.h"
#include "rTypes.h"
#include "reGpio.h"
#include "reLed.h"
#include "reEvents.h"
#include "reParams.h"
#include "rLog.h"

static const char* logTAG = "ALARM";

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------- Проводные входы ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// Объекты reGPIO для обработки прерываний по проводным входым ОПС
static reGPIO gpioAlarm1(CONFIG_GPIO_ALARM_ZONE_1, CONFIG_GPIO_ALARM_LEVEL, false, true, CONFIG_BUTTON_DEBOUNCE_TIME_US, nullptr);
static reGPIO gpioAlarm2(CONFIG_GPIO_ALARM_ZONE_2, CONFIG_GPIO_ALARM_LEVEL, false, true, CONFIG_BUTTON_DEBOUNCE_TIME_US, nullptr);
static reGPIO gpioAlarm3(CONFIG_GPIO_ALARM_ZONE_3, CONFIG_GPIO_ALARM_LEVEL, false, true, CONFIG_BUTTON_DEBOUNCE_TIME_US, nullptr);
static reGPIO gpioAlarm4(CONFIG_GPIO_ALARM_ZONE_4, CONFIG_GPIO_ALARM_LEVEL, false, true, CONFIG_BUTTON_DEBOUNCE_TIME_US, nullptr);
static reGPIO gpioAlarm5(CONFIG_GPIO_ALARM_ZONE_5, CONFIG_GPIO_ALARM_LEVEL, false, true, CONFIG_BUTTON_DEBOUNCE_TIME_US, nullptr);

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- Инициализация ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void alarmInitDevices()
{
  rlog_i(logTAG, "Initialization of AFS devices");

  // Создаем светодиоды, сирену и флешер
  ledQueue_t ledAlarm = nullptr;
  #if defined(CONFIG_GPIO_ALARM_LED) && (CONFIG_GPIO_ALARM_LED > -1)
    ledAlarm = ledTaskCreate(CONFIG_GPIO_ALARM_LED, true, true, "led_alarm", CONFIG_LED_TASK_STACK_SIZE, nullptr);
    ledTaskSend(ledAlarm, lmOff, 0, 0, 0);
  #endif // CONFIG_GPIO_ALARM_LED
  ledQueue_t siren = nullptr;
  #if defined(CONFIG_GPIO_ALARM_SIREN) && (CONFIG_GPIO_ALARM_SIREN > -1)
    siren = ledTaskCreate(CONFIG_GPIO_ALARM_SIREN, true, false, "siren", CONFIG_LED_TASK_STACK_SIZE, nullptr);
    ledTaskSend(siren, lmOff, 0, 0, 0);
  #endif // CONFIG_GPIO_ALARM_SIREN
  ledQueue_t flasher = nullptr;
  #if defined(CONFIG_GPIO_ALARM_FLASH) && (CONFIG_GPIO_ALARM_FLASH > -1)
    flasher = ledTaskCreate(CONFIG_GPIO_ALARM_FLASH, true, true, "flasher", CONFIG_LED_TASK_STACK_SIZE, nullptr);
    ledTaskSend(flasher, lmBlinkOn, 1, 100, 5000);
  #endif // CONFIG_GPIO_ALARM_FLASH
  
  // Замена пассивной пищалки на активную
  ledQueue_t buzzer = nullptr;
  #if defined(CONFIG_GPIO_BUZZER_ACTIVE) && (CONFIG_GPIO_BUZZER_ACTIVE > -1)
    buzzer = ledTaskCreate(CONFIG_GPIO_BUZZER_ACTIVE, true, false, "buzzer", CONFIG_LED_TASK_STACK_SIZE, nullptr);
    ledTaskSend(buzzer, lmOff, 0, 0, 0);
  #endif // CONFIG_GPIO_ALARM_FLASH
  
  // Запускаем задачу
  alarmTaskCreate(siren, flasher, buzzer, ledAlarm, ledAlarm, nullptr);

  // Запускаем приемник RX 433 MHz
  #ifdef CONFIG_GPIO_RX433
    rx433_Init(CONFIG_GPIO_RX433, alarmTaskQueue());
    rx433_Enable();
  #endif // CONFIG_GPIO_RX433
}

void alarmInitSensors()
{
  rlog_i(logTAG, "Initialization of AFS zones");

  // -----------------------------------------------------------------------------------
  // Настраиваем зоны охраны
  // -----------------------------------------------------------------------------------

  // Двери (периметр) :: создаем зону охраны
  alarmZoneHandle_t azDoors = alarmZoneAdd(
    "Двери",           // Понятное название зоны
    "doors",           // MQTT-топик зоны
    nullptr            // Функция управления реле, при необходимости
  );
  // Настраиваем реакции для данной зоны в разных режимах
  alarmResponsesSet(
    azDoors,           // Ссылка на зону охраны
    ASM_DISABLED,      // Настраиваем реакции для режима ASM_DISABLED - "охрана отключена"
    ASRS_REGISTER,     // Реакция на события тревоги: только регистрация (фактически это приводит к публикации его на MQTT)
    ASRS_REGISTER      // Реакция на отмену тревоги: только регистрация (фактически это приводит к публикации его на MQTT)
  );
  alarmResponsesSet(
    azDoors,           // Ссылка на зону охраны
    ASM_ARMED,         // Настраиваем реакции для режима ASM_ARMED - "полная охрана"
    ASRS_ALARM_SIREN,  // Реакция на события тревоги: включить сирену и отправить уведомления
    ASRS_REGISTER      // Реакция на отмену тревоги: только регистрация (фактически это приводит к публикации его на MQTT)
  );
  alarmResponsesSet(
    azDoors,           // Ссылка на зону охраны
    ASM_PERIMETER,     // Настраиваем реакции для режима ASM_PERIMETER - "только периметр (дома)" 
    ASRS_ALARM_SIREN,  // Реакция на события тревоги: включить сирену и отправить уведомления
    ASRS_REGISTER      // Реакция на отмену тревоги: только регистрация (фактически это приводит к публикации его на MQTT)
  );
  alarmResponsesSet(
    azDoors,           // Ссылка на зону охраны
    ASM_OUTBUILDINGS,  // Настраиваем реакции для режима ASM_OUTBUILDINGS - "внешние помещения" 
    ASRS_ALARM_NOTIFY, // Реакция на события тревоги: тихая тревога - отправить уведомления, но сирену не включать
    ASRS_REGISTER      // Реакция на отмену тревоги: только регистрация (фактически это приводит к публикации его на MQTT)
  );

  // Окна (периметр)
  alarmZoneHandle_t azWindows = alarmZoneAdd("Окна", "windows", nullptr);
  alarmResponsesSet(azWindows, ASM_DISABLED, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azWindows, ASM_ARMED, ASRS_ALARM_SIREN, ASRS_REGISTER);
  alarmResponsesSet(azWindows, ASM_PERIMETER, ASRS_ALARM_NOTIFY, ASRS_REGISTER);
  alarmResponsesSet(azWindows, ASM_OUTBUILDINGS, ASRS_ALARM_NOTIFY, ASRS_REGISTER);

  // Дом (внутренние помещения)
  alarmZoneHandle_t azIndoor = alarmZoneAdd("Дом", "indoor", nullptr);
  alarmResponsesSet(azIndoor, ASM_DISABLED, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azIndoor, ASM_ARMED, ASRS_ALARM_SIREN, ASRS_REGISTER);
  alarmResponsesSet(azIndoor, ASM_PERIMETER, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azIndoor, ASM_OUTBUILDINGS, ASRS_REGISTER, ASRS_REGISTER);

  // Двор (внешние датчики - только уведомления, без сирен и тревоги)
  alarmZoneHandle_t azOutdoor = alarmZoneAdd("Двор", "outdoor", nullptr);
  alarmResponsesSet(azOutdoor, ASM_DISABLED, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azOutdoor, ASM_ARMED, ASRS_ALARM_NOTIFY, ASRS_REGISTER);
  alarmResponsesSet(azOutdoor, ASM_PERIMETER, ASRS_ALARM_NOTIFY, ASRS_REGISTER);
  alarmResponsesSet(azOutdoor, ASM_OUTBUILDINGS, ASRS_ALARM_NOTIFY, ASRS_REGISTER);

  // Датчики дыма и пламени - тревога 24*7
  alarmZoneHandle_t azFire = alarmZoneAdd("Пожар", "fire", nullptr);
  alarmResponsesSet(azFire, ASM_DISABLED, ASRS_ALARM_SILENT, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azFire, ASM_ARMED, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azFire, ASM_PERIMETER, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azFire, ASM_OUTBUILDINGS, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  
  // Tamper (попытки вскрытия системы)
  alarmZoneHandle_t azTamper = alarmZoneAdd("Tamper", "tamper", nullptr);
  alarmResponsesSet(azTamper, ASM_DISABLED, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTamper, ASM_ARMED, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTamper, ASM_PERIMETER, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTamper, ASM_OUTBUILDINGS, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);

  // Контроль сетевого напряжения
  alarmZoneHandle_t azPower = alarmZoneAdd("Контроль питания", "power", nullptr);
  alarmResponsesSet(azPower, ASM_DISABLED, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azPower, ASM_ARMED, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azPower, ASM_PERIMETER, ASRS_REGISTER, ASRS_REGISTER);
  alarmResponsesSet(azPower, ASM_OUTBUILDINGS, ASRS_REGISTER, ASRS_REGISTER);

  // Инженерные системы: протечка воды, утечка газа, датчик(и) угарного газа и т.д.
  alarmZoneHandle_t azTech = alarmZoneAdd("Инженерные системы", "tech", nullptr);
  alarmResponsesSet(azTech, ASM_DISABLED, ASRS_ALARM_SILENT, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTech, ASM_ARMED, ASRS_ALARM_SILENT, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTech, ASM_PERIMETER, ASRS_ALARM_SILENT, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azTech, ASM_OUTBUILDINGS, ASRS_ALARM_SILENT, ASRS_ALARM_NOTIFY);

  // Тревожные кнопки
  alarmZoneHandle_t azButtons = alarmZoneAdd("Тревожные кнопки", "buttons", nullptr);
  alarmResponsesSet(azButtons, ASM_DISABLED, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azButtons, ASM_ARMED, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azButtons, ASM_PERIMETER, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);
  alarmResponsesSet(azButtons, ASM_OUTBUILDINGS, ASRS_ALARM_SIREN, ASRS_ALARM_NOTIFY);

  // 433 MHz пульты управления
  alarmZoneHandle_t azRemoteControls = alarmZoneAdd("Пульты управления", "controls", nullptr);
  alarmResponsesSet(azRemoteControls, ASM_DISABLED, ASRS_CONTROL, ASRS_CONTROL);
  alarmResponsesSet(azRemoteControls, ASM_ARMED, ASRS_CONTROL, ASRS_CONTROL);
  alarmResponsesSet(azRemoteControls, ASM_PERIMETER, ASRS_CONTROL, ASRS_CONTROL);
  alarmResponsesSet(azRemoteControls, ASM_OUTBUILDINGS, ASRS_CONTROL, ASRS_CONTROL);

  rlog_i(logTAG, "Initialization of AFS sensors");

  // -----------------------------------------------------------------------------------
  // Проводные входы для встроенных GPIO
  // -----------------------------------------------------------------------------------

  // Проводная зона 1: входная дверь
  gpioAlarm1.initGPIO();
  alarmSensorHandle_t asWired1 = alarmSensorAdd(
    AST_WIRED,                                      // Тип датчика: проводные датчики
    "Входная дверь",                                // Понятное имя датчика
    "door",                                         // Топик датчика
    CONFIG_ALARM_LOCAL_PUBLISH,                     // Использовать локальные топики для передачи сигналов на другие устройства, в примере = TRUE (0x01)
    CONFIG_GPIO_ALARM_ZONE_1                        // Номер вывода или адрес датчика
  );
  if (asWired1) {
    alarmEventSet(asWired1, azDoors, 0, ASE_ALARM, 
      1, CONFIG_ALARM_EVENT_MESSAGE_DOOROPEN,       // Сообщение при сигнале тревоги: "🚨 Открыта дверь"
      0, NULL,                                      // Сообщение при отмене тревоги: отсутствует
      1,                                            // Порог срабатывания (нужен только для беспроводных датчиков, для остальных = 1)
      0,                                            // Время автосброса тревоги по таймеру, 0 = отключено
      60,                                           // Период публикации на MQTT брокере
      false);                                       // Тревога без подтверждения с других датчиков
  };
 
  // Проводная зона 2: PIR сенсор в прихожей
  gpioAlarm2.initGPIO();
  alarmSensorHandle_t asWired2 = alarmSensorAdd(AST_WIRED, "Прихожая", "hallway", CONFIG_ALARM_LOCAL_PUBLISH, CONFIG_GPIO_ALARM_ZONE_2);
  if (asWired2) {
    alarmEventSet(asWired2, azIndoor, 0, ASE_ALARM, 
      1, CONFIG_ALARM_EVENT_MESSAGE_MOTION,         // Сообщение при сигнале тревоги: "🚨 Обнаружено движение"
      0, NULL,                                      // Сообщение при отмене тревоги: отсутствует
      1,                                            // Порог срабатывания (нужен только для беспроводных датчиков, для остальных = 1)
      0,                                            // Время автосброса тревоги по таймеру, 0 = отключено (это проводной PIR, он сам все умеет)
      60,                                           // Период публикации на MQTT брокере
      true);                                        // Тревогу поднимать только при подтверждении с других иди этого же датчика - PIR иногда могут выдавать ложные тревоги 
  };

  // Проводная зона 3: 
  gpioAlarm3.initGPIO();
  alarmSensorHandle_t asGasLeak = alarmSensorAdd(AST_WIRED, "Газ", "gas", CONFIG_ALARM_LOCAL_PUBLISH, CONFIG_GPIO_ALARM_ZONE_3);
  if (asGasLeak) {
    alarmEventSet(asGasLeak, azTech, 0, ASE_ALARM, 
      1, CONFIG_ALARM_EVENT_MESSAGE_GAS,            // Сообщение при сигнале тревоги: "🚨 Обнаружена утечка газа"
      0, CONFIG_ALARM_EVENT_MESSAGE_CLEAR,          // Сообщение при отмене тревоги: "🟢 Авария устранена"
      1,                                            // Порог срабатывания (нужен только для беспроводных датчиков, для остальных = 1)
      0,                                            // Время автосброса тревоги по таймеру, 0 = отключено
      60,                                           // Период публикации на MQTT брокере
      false);                                       // Тревога без подтверждения с других датчиков
  };

  // Проводная зона 4: контроль питания 220В
  gpioAlarm4.initGPIO();
  alarmSensorHandle_t asPowerMain = alarmSensorAdd(AST_WIRED, "Питание 220В", "main_power", CONFIG_ALARM_LOCAL_PUBLISH, CONFIG_GPIO_ALARM_ZONE_4);
  if (asPowerMain) {
    alarmEventSet(asPowerMain, azPower, 0, ASE_POWER, 
      1, CONFIG_ALARM_EVENT_MESSAGE_POWER_MAIN_OFF, // Сообщение при сигнале тревоги: "🔴 Основное питание отключено"
      0, CONFIG_ALARM_EVENT_MESSAGE_POWER_MAIN_ON,  // Сообщение при отмене тревоги: "💡 Основное питание восстановлено"
      1,                                            // Порог срабатывания (нужен только для беспроводных датчиков, для остальных = 1)
      0,                                            // Время автосброса тревоги по таймеру, 0 = отключено
      0,                                            // Без повторной публикации состояния
      false);                                       // Тревога без подтверждения с других датчиков
  };

  // Проводная зона 5: контроль заряда аккумулятора
  gpioAlarm5.initGPIO();
  alarmSensorHandle_t asBattery = alarmSensorAdd(AST_WIRED, "Аккумулятор", "battery", false, CONFIG_GPIO_ALARM_ZONE_5);
  if (asBattery) {
    alarmEventSet(asBattery, azPower, 0, ASE_POWER, 
      0, CONFIG_ALARM_EVENT_MESSAGE_BATTERY_LOW,    // Сообщение при сигнале тревоги: "🔋 Низкий уровень заряда батареи"
      1, CONFIG_ALARM_EVENT_MESSAGE_BATTERY_NRM,    // Сообщение при отмене тревоги: "🔋 Аккумулятор заряжен"
      1,                                            // Порог срабатывания (нужен только для беспроводных датчиков, для остальных = 1)
      0,                                            // Время автосброса тревоги по таймеру, 0 = отключено
      0,                                            // Без повторной публикации состояния
      false);                                       // Тревога без подтверждения с других датчиков
  };

  // -----------------------------------------------------------------------------------
  // Беспроводные датчики 433 МГц
  // -----------------------------------------------------------------------------------

  // Датчик дыма (код 0x00D77979)
  alarmSensorHandle_t asSmokeFE = alarmSensorAdd(AST_RX433_20A4C, "Дым", "smoke", false, 0x000D7797);
  if (asSmokeFE) {
    alarmEventSet(asSmokeFE, azFire, 0, ASE_ALARM, 
      0x09, CONFIG_ALARM_EVENT_MESSAGE_SMOKE,       // Сообщение при сигнале тревоги: "🔥 Обнаружено задымление"
      ALARM_VALUE_NONE, NULL,                       // Сообщение при отмене тревоги: отсутствует
      1,                                            // Порог срабатывания: при первом же сигнале
      60*1000,                                      // Время автосброса: 60 секунд
      0,                                            // Без повторной публикации состояния
      false);                                       // Тревога без подтверждения с других датчиков
  };

  // PIR комната 1 (maxkin PIR-600)
  alarmSensorHandle_t asPirCorridor = alarmSensorAdd(
    AST_RX433_20A4C,                                // Тип датчика: беспроводной
    "Комната 1",                                    // Понятное имя датчика
    "room1/pir",                                    // Топик датчика
    CONFIG_ALARM_LOCAL_PUBLISH,                     // Использовать локальные топики для передачи сигналов на другие устройства, в примере = TRUE (0x01) 
    0x0004D1D0                                      // Адрес датчика
  );
  if (asPirCorridor) {
    // Первая команда
    alarmEventSet(asPirCorridor, azIndoor, 0, 
      ASE_ALARM,                                    // Основная команда - движение, её код 0x09
      0x09, CONFIG_ALARM_EVENT_MESSAGE_MOTION,      // Сообщение при сигнале тревоги: "🚨 Обнаружено движение"
      ALARM_VALUE_NONE, NULL,                       // Сообщение при отмене тревоги: отсутствует
      1,                                            // Порог срабатывания: при первом же сигнале
      30*1000,                                      // Время автосброса: 30 секунд
      600,                                          // Период публикации на MQTT брокере
      true);                                        // Тревогу поднимать только при подтверждении с других иди этого же датчика - PIR иногда могут выдавать ложные тревоги 
    // Вторая команда
    alarmEventSet(asPirCorridor, azTamper, 1,   
      ASE_TAMPER,                                   // Команда tamper, её код 0x0D
      0x0D, CONFIG_ALARM_EVENT_MESSAGE_TAMPER,      // Сообщение при сигнале тревоги: "⚠️ Попытка взлома датчика"
      ALARM_VALUE_NONE, NULL,                       // Сообщение при отмене тревоги: отсутствует
      1,                                            // Порог срабатывания: при первом же сигнале
      5*60*1000,                                    // Время автосброса: 5 минут
      0,                                            // Без повторной публикации состояния
      false);                                       // Тревога без подтверждения с других датчиков
  };
 
  // -----------------------------------------------------------------------------------
  // пульты управления 433 МГц
  // -----------------------------------------------------------------------------------

  alarmSensorHandle_t asRC_R2 = alarmSensorAdd(     
    AST_RX433_20A4C,                                // Тип датчика: беспроводной
    "Пульт",                                        // Название пульта
    "rc",                                           // Топик пульта
    false,                                          // Локальные топики не используются
    0x0004F9CB                                      // Адрес пульта
  );
  if (asRC_R2) {
    alarmEventSet(asRC_R2, azRemoteControls, 0,     // Зона "пультов"
      ASE_CTRL_OFF,                                 // Команда отключения режима охраны
      0x01, NULL,                                   // Код команды 0x01, без сообщений
      ALARM_VALUE_NONE, NULL,                       // Кода отмены нет, без сообщений
      2,                                            // Должно придти как минимум 2 кодовых посылки для переключения
      3*1000,                                       // Время автосброса: 3 секунды
      0,                                            // Без повторной публикации состояния
      false);                                       // Не требуется подтвеждение с других датчиков
    alarmEventSet(asRC_R2, azRemoteControls, 1,     // Зона "пультов"
      ASE_CTRL_ON,                                  // Команда включения режима охраны
      0x08, NULL,                                   // Код команды 0x08, без сообщений
      ALARM_VALUE_NONE, NULL,                       // Кода отмены нет, без сообщений
      2,                                            // Должно придти как минимум 2 кодовых посылки для переключения
      3*1000,                                       // Время автосброса: 3 секунды
      0,                                            // Без повторной публикации состояния
      false);                                       // Не требуется подтвеждение с других датчиков
    alarmEventSet(asRC_R2, azRemoteControls, 2,     // Зона "пультов"
      ASE_CTRL_PERIMETER,                           // Команда включения режима "периметр"
      0x04, NULL,                                   // Код команды 0x04, без сообщений
      ALARM_VALUE_NONE, NULL,                       // Кода отмены нет, без сообщений
      2,                                            // Должно придти как минимум 2 кодовых посылки для переключения
      3*1000,                                       // Время автосброса: 3 секунды
      0,                                            // Без повторной публикации состояния
      false);                                       // Не требуется подтвеждение с других датчиков
    alarmEventSet(asRC_R2, azButtons, 3,            // Зона "тревожные кнопки"
      ASE_ALARM,                                    // Команда "тревога"
      0x02, NULL,                                   // Код команды 0x02, сообщение "🔴 Нажата тревожная кнопка"
      ALARM_VALUE_NONE, NULL,                       // Кода отмены нет, без сообщений
      2,                                            // Должно придти как минимум 2 кодовых посылки для переключения
      3*1000,                                       // Время автосброса: 3 секунды
      0,                                            // Без повторной публикации состояния
      false);                                       // Не требуется подтвеждение с других датчиков
  };

  rlog_i(logTAG, "Initialization of AFS completed");
}

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------- Внешние датчики ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
  
static bool _extToiletPir  = false;
static bool _extKitchenPir = false;

#define EXT_DATA_QOS                  2
#define EXT_DATA_FRIENDLY             "Состояние"

#define EXT_DATA_TOILET_PIR_ID         0xFF000001
#define EXT_DATA_TOILET_PIR_KEY       "toilet_pir"
#define EXT_DATA_TOILET_PIR_TOPIC     "security/home/toilet/pir"     // local/security/home/toilet/pir/status
#define EXT_DATA_TOILET_PIR_FRIENDLY  "Санузел"

#define EXT_DATA_KITCHEN_PIR_ID        0xFF000002
#define EXT_DATA_KITCHEN_PIR_KEY      "kitchen_pir"
#define EXT_DATA_KITCHEN_PIR_TOPIC    "security/home/kitchen/pir"     // local/security/home/kitchen/pir/status
#define EXT_DATA_KITCHEN_PIR_FRIENDLY "Кухня"

static void alarmExternalSensorsEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (*(uint32_t*)event_data == (uint32_t)&_extToiletPir) {
    if ((event_id == RE_PARAMS_CHANGED) || (event_id == RE_PARAMS_EQUALS)) {
      vTaskDelay(1);
      alarmPostQueueExtId(IDS_MQTT, EXT_DATA_TOILET_PIR_ID, _extToiletPir);
    };
  } else if (*(uint32_t*)event_data == (uint32_t)&_extKitchenPir) {
    if ((event_id == RE_PARAMS_CHANGED) || (event_id == RE_PARAMS_EQUALS)) {
      vTaskDelay(1);
      alarmPostQueueExtId(IDS_MQTT, EXT_DATA_KITCHEN_PIR_ID, _extKitchenPir);
    };
  };
}

static void alarmExternalSensorsInit()
{
  paramsGroupHandle_t extDataToilet = paramsRegisterGroup(nullptr, 
    EXT_DATA_TOILET_PIR_KEY, EXT_DATA_TOILET_PIR_TOPIC, EXT_DATA_TOILET_PIR_FRIENDLY);
  if (extDataToilet) {
    paramsRegisterValue(OPT_KIND_LOCDATA_ONLINE, OPT_TYPE_U8, nullptr, extDataToilet, 
      CONFIG_ALARM_MQTT_EVENTS_STATUS, EXT_DATA_FRIENDLY, EXT_DATA_QOS, &_extToiletPir);
  };

  paramsGroupHandle_t extDataKitchen = paramsRegisterGroup(nullptr, 
    EXT_DATA_KITCHEN_PIR_KEY, EXT_DATA_KITCHEN_PIR_TOPIC, EXT_DATA_KITCHEN_PIR_FRIENDLY);
  if (extDataKitchen) {
    paramsRegisterValue(OPT_KIND_LOCDATA_ONLINE, OPT_TYPE_U8, nullptr, extDataKitchen, 
      CONFIG_ALARM_MQTT_EVENTS_STATUS, EXT_DATA_FRIENDLY, EXT_DATA_QOS, &_extKitchenPir);
  };

  eventHandlerRegister(RE_PARAMS_EVENTS, RE_PARAMS_CHANGED, alarmExternalSensorsEventHandler, nullptr);
  eventHandlerRegister(RE_PARAMS_EVENTS, RE_PARAMS_EQUALS, alarmExternalSensorsEventHandler, nullptr);
}


void alarmStart()
{
  alarmInitDevices();
  alarmExternalSensorsInit();
  alarmInitSensors();
}