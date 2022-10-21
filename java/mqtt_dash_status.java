if (event.getLastPayload() == 'OFFLINE') {
  event.textColor = '#ff0000';
  event.blink = true;
  event.text = 'Устройство выключено или offline';
} else {
  event.textColor = '#9acd32';
  event.blink = false;
};