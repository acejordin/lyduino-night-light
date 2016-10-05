# lyduino-night-light
A sketch for Arduino Yun for a dual configurable night light

Idea here is that the kiddo is asleep by the time the night light turns off at night and can get up when the day light turns in the morning.

### Future improvements
Add webserver and website to allow configuring start/stop times for each light over wifi

### Modify these lines to configure the time each light comes on and turns off.
Note: The hour should be in military time (0-23).
```
//Night light time
//Night Start Time (Military time)
int nightStartHour = 19;
int nightStartMinute = 55;

//Night End Time (Military time)
int nightEndHour = 20;
int nightEndMinute = 30;

//Day light time
//Day Start Time (Military time)
int dayStartHour = 6;
int dayStartMinute = 30;

//Day End Time (Military time)
int dayEndHour = 8;
int dayEndMinute = 00;
```
