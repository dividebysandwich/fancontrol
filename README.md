# fancontrol

This is a two-channel fan controller for basement and roof ventilation systems. The principle is simple: An extractor fan pulls out hot air, and an intake fan pulls in cool and dry air. To avoid pulling in humid air, two sensors compare the climate inside and outside, and only activate the intake fan if the exterior air is less humid than the air inside.
I have been looking for something off the shelf for this task, but most humidity-controlled fans just take a single reading inside and decide whether to run based on that. Needless to say, that's suboptimal. More complete solutions exist, but cost in excess of several hundred dollars. So I made this ;)

![Fan controller in action](https://i.imgur.com/Xxy3dso.png)

## WARNING

This is a mains project. Make sure you know what you are doing. Proceed at your own risk.

## Parts list

  * 1x ESP32 Dev Board (https://www.espressif.com/en/products/hardware/esp32-devkitc/overview - cheap clones can be found on amazon, banggood, etc.)
  * 1x SainSmart dual-channel relay board (https://www.sainsmart.com/products/2-channel-5v-relay-module)
  * 1x SI7021 sensor (https://learn.sparkfun.com/tutorials/si7021-humidity-and-temperature-sensor-hookup-guide) or similar.
  * 1x I2C Multiplexer (https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/wiring-and-test) or similar.
  * 1x Mini power supply (For example https://www.aliexpress.com/item/Free-Shipping-1pcs-lot-HLK-PM01-AC-DC-220V-to-5V-mini-power-supply-module-intelligent/32408565688.html)
  * 1x HD44780 2004 LCD Display Bundle (https://www.amazon.de/AZDelivery-HD44780-Display-Zeichen-Schnittstelle/dp/B07DDGNPX1 - make sure the I2C interface board is included!) 
  * A fitting case
  * Mains cabling
  * Two mains voltage wall mount / in-line fans of your choice.

## Wiring

I am too lazy for a diagram. Basically, you have to solder the I2C board onto the LCD, and connect it the grounds and VCC (5V) on the ESP32. 
Connect the I2C multiplexer to GND and 3.3V (NOT 5V!) as well as the I2C data lines SDA (Pin 27 on ESP32) and SCL (Pin 14 on ESP32).
The LCDs SDA/SCL is connected to the SD0/SC0 pins on the multiplexer, and the SI7021 sensor connects to SD1/SC1. 
The relay board has two signal pins for the two channels, which connect to pin 5 and 13 on the ESP32. Finally, connect a momentary pushbutton between ground and Pin 32 on the ESP32.
That's it! All that's left to do is to wire the mains for the fans through the relay contacts.

## Libraries used

This project uses LiquidCrystal_I2C and DHTesp, install them via the library manager in the Arduino IDE. Do NOT use the adafruit DHT library, it will only cause you grief.

## Configuration
Check the top of the source for Wifi variables. Basically there is an optional sending of data to another webserver which you can remove if not needed. External temperature and humidity are fetched from another webserver, the data is expected to be inside a simple text file which is web-readable. I'm using this to fetch data from an outside weather station. This reduces complexity because there's no need to worry about I2C over a long wire, but of course you need Wifi for that. And an external weather sensor. But since there's a multiplexer connected, you could add your own second sensor. However you'll likely still need the multiplexer even if you don't intend to do that, because apparently the LCD library does not play well with other I2C devices on the same bus.

## Operation
Everything is automatic. Feel free to adapt the values and/or logic in the code to your preference. The controller will display new values every second, but the fans will change state only once per minute. This is to avoid rapidy cycling the fans on and off, and to allow sensor values to settle. For some reason the LCD may not always initialize on powerup. It will however work after a reset - hence the ugly reset button. If someone figures out why this happens and how to fix it, please let me know.



