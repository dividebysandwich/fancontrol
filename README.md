# fancontrol

This is a two-channel fan controller for basement and roof ventilation systems. The principle is simple: An extractor fan pulls out hot air, and an intake fan pulls in cool and dry air. To avoid pulling in humid air, two sensors compare the climate inside and outside, and only activate the intake fan if the exterior air is less humid than the air inside.
I have been looking for something off the shelf for this task, but most humidity-controlled fans just take a single reading inside and decide whether to run based on that. Needless to say, that's suboptimal. More complete solutions exist, but cost in excess of several hundred dollars. So I made this ;)

![Fan controller in action](https://i.imgur.com/Xxy3dso.png)

## WARNING

This is a mains project. Make sure you know what you are doing. Proceed at your own risk.

## Parts list

  * 1x ESP32 Dev Board (https://www.espressif.com/en/products/hardware/esp32-devkitc/overview - cheap clones can be found on amazon, banggood, etc.)
  * 1x SainSmart dual-channel relay board (https://www.sainsmart.com/products/2-channel-5v-relay-module)
  * 2x DHT22 sensor (https://www.adafruit.com/product/385) or similar.
  * 1x Mini power supply (For example https://www.aliexpress.com/item/Free-Shipping-1pcs-lot-HLK-PM01-AC-DC-220V-to-5V-mini-power-supply-module-intelligent/32408565688.html)
  * 1x HD44780 2004 LCD Display Bundle (https://www.amazon.de/AZDelivery-HD44780-Display-Zeichen-Schnittstelle/dp/B07DDGNPX1 - make sure the I2C interface board is included!) 
  * Some case
  * Mains cabling
  * Two mains voltage wall mount / in-line fans of your choice.

## Wiring

I am too lazy for a diagram. Basically, you have to solder the I2C board onto the LCD, and connect all the grounds and VCCs. I put the DHT22 on +5V to make sure it gets enough voltage at the end of a long cable.
Then, connect the LCDs SDA to pin 27 on the ESP32, SCL connects to pin 14. The relay board has two signal pins for the two relay, which connect to pin 5 and 13 on the ESP32. Finally, the data pin of the DHT sensors (second from left) connects to pin 25 and 26 on the ESP32.
That's it! All that's left to do is to wire the mains for the fans through the relay contacts.

## Libraries used

This project uses LiquidCrystal_I2C and DHTesp, install them via the library manager in the Arduino IDE. Do NOT use the adafruit DHT library, it will only cause you grief.

## Operation
Everything is automatic. Feel free to adapt the values and/or logic in the code to your preference. The controller will display new values every three seconds, but the fans will change state only once every 1.5 minutes or so. This is to avoid rapidy cycling the fans on and off, and to allow sensor values to settle.

