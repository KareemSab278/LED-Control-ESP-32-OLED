i created this because the lib 'RS_WS281X' works on the pi4b but not the 5.
something about 'hardware failure'.

so i decided to go ahead with the esp-32 and send it commands over usb serial.
its a quick fix but it is what it is i guess.

i prefer it this way regardless for simplicity in how the gpio works with the raspberry pi.
a simple plug and play esp-32 for led control proves easy for setup anyway.

if you are working on this project, please ensure that you do not have brightness set too high.
30 is okay enough, otherwise youll get power problems and the leds will not work.
