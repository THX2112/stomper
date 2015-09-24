# Stomper
## Eurorack Drum Module with Clock Divider
 
Information here: http://syinsi.com/projects/stomper

Uses AtMega328p-pu @ 16MHz. 

The Stomper firmware is updated through the FTDI port with a USB Arduino Adapter. 

Connect DTR to CTS, RX to TX, TX to RX, and GND to GND.

Download the latest firmware from here and put it into your Arduino sketch directory.

Load it with the Arduino IDE. Set Tools>Board to “Arduino Uno”, and Tools>Serial Port to the port the USB FTDI interface is using (the Arduino IDE usually figures this out for itself.) Click the “Upload” button and it should compile and transfer.
