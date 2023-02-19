# Термостат + охранно-пожарная сигнализация

Проект WiFi термостата и охранно-пожарной сигнализации на ESP32 и ESP-IDF. Полное описание смотрите на канале [dzen.ru/kotyara12](https://dzen.ru/kotyara12)
Вы можете скачать любую необходимую ветвь прошивки, описание ветвей ниже

- **01_telemeter** - Телеметрия и контроль диапазонов температуры (без функций управления котлом, только постоянный мониторинг и уведомления). Только для версии ESP-IDF 4.4.x
- **02_telemeter_charts** - То же самое, но добавлена отправка данных на вненние сервисы. Только для версии ESP-IDF 4.4.x!
- **03_ESP-IDF_5-0-0** - Библиотеки и сам проект адаптированы под ESP-IDF новой версии 5.0.0 и выше. **Но будьте внимательны - в самой ESP-IDF пока есть неимправленные проблемы.**. Данная версия одиаково работает и с ESP-IDF 4.4 и с ESP-IDF 5.0, версия определяется автоматически (макросами)
- **master** - Последняя актуальная ветка. Если вам не нужна какая-то спецальная версия, смело берите её.


# Как этим пользоваться
1. Создайте на ```диске C``` каталог ```PlatformIO```, то есть ```C:\PlatformIO\```. Вы можете использовать любой другой диск и каталог, но в этом случае вам придется изменять настройки в нескольких файлах конфигурации. Если вы не готовы к этому, то оставьте "как есть", то есть ```C:\PlatformIO\```.
2. Скачайте нужную вам ветку данного репозитория в виде ZIP-архива и распакуйте его. Вы получите каталог с файлами в виде ```telemeter_dzen-%branches%```, например ```telemeter_dzen-01_telemeter```. Переименуйте его в просто ```telemeter_dzen``` и переместите в ```C:\PlatformIO\```. У вас должно получиться так: ```C:\PlatformIO\telemeter_dzen```.
3. Внутри ```C:\PlatformIO\telemeter_dzen``` вы найдете файл ```libs_local_20221010.rar``` (или аналогичный) - распакуйте его в каталог ```C:\PlatformIO\```. У вас должно получиться так: ```C:\PlatformIO\libs``` и внутри много подкаталогов с библиотеками.

Всё готово, можно пробовать открывать проект в VS Code и пробовать компилировать.
