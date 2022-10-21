function twoDig(value) {
  if (value>9) {
    return value;
  } else {
    return "0"+value;
  };
};

var total=event.text;
var secs=total;
var hour=Math.trunc(secs/3600);
secs=secs-hour*3600;
var mins=Math.trunc(secs/60);
secs=secs-mins*60;
event.text=twoDig(hour)+":"
          +twoDig(mins)+":"
          +twoDig(secs)+"\n"
          +Math.round(10000.0*total/(3600*24))/100+"%\n"
          +Math.round(total*19/3600)/1000+" кВт/ч";