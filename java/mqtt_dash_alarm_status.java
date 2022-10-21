var sJSON = event.getLastPayload();
if (sJSON != '') {
  var data = JSON.parse(sJSON);

  // Формируем текст состояния из трех строк: статус, 
  // последний активный сенсор (или пульт) и время его сработки
  event.text = data['status'] + '\n'
             + data['event']['sensor'] + '\n'
             + data['event']['time_short'];

  var iMode = data['mode'];
  if (iMode == 0) {
    // Охрана отключена
    if (data['annunciator']['summary'] > 0) {
      // Тревога по зонам 24 в настоящий момент
      event.textColor = '#FF0000';
      event.blink = true;
    } else {
      // Тревоги нет, всё тихо
      event.textColor = '#9ACD32';
      event.blink = false;
    };
  } else {
    // Мигание, если были зафиксированы тревоги с момента последнего включения
    if ((data['alarms'] > 0) || (data['annunciator']['summary'] > 0)) {
      event.textColor = '#FF0000';
      event.blink = true;
    } else {
      event.textColor = '#FFFF00';
      event.blink = false;
    };
  };
} else {
  // Нет данных
  event.text = 'Устройство выключено или не доступно';
  event.textColor = '#FF0000';
  event.blink = true;
};