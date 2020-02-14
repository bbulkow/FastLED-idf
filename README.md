# FastLED-idf

The ESP32 is a pretty great package. It has two
cores, 240Mhz speed, low power mode, wifi and bluetooth, 
and can be had in pre-built modules at prices between $24 ( Adafruit, Sparkfun ), $10 (Espressif-manufactured boards through Mauser and Digikey),
or 'knock off' boards for $4 from AliExpress.

If you're going to program this board, you might use Arduino.
Although I love the concept of Arduino - and the amazing amount
of libraries - the reality is the Arduino IDE is a mess, and the compile
environment is "funky", that is, it's not really C - about my first
minute in the IDE, I managed to write a perfectly valid C preprocessor
directive that's illegal in Arduino.

Enter ESP-IDF, which is Espressif's RTOS. It's based on FreeRTOS,
but they had to fork it for multiple cores. It's based on FreeRTOS 9,
but has some of the work done later backported.

There are a TON of useful modules included with ESP-IDF. Notably,
nghttp server, mdns, https servers, websockets, json, mqtt, etc etc.

What I tend to do with embedded systems is blink LEDs! Sure, there's other
fun stuff, but blinking LEDs is pretty good.

Thus, we need FastLED.

# The other FastLED-idf

It appears that set of code was an earlier version of FastLED, 
and probably only still works with ESP-IDF 3.x. There was a major
update to ESP-IDF, a new build system, lots of submodule updates,
cleaned up headers and names, which seem to be all for the better -
but with lots of breaking changes.

Thus, updating both, and using eskrab's version as a template, 
I attempt to have a running version of FastLED with the ESP-IDF
4.0 development environment

# ESP-IDF already has a LED library

Yes, I saw it once, I'm having trouble finding it now. It does
really cool timing tricks, but it's not got all the fancy-fancy
of the FastLED library, which has a better designed and time tested 
programming interface. I reached out on the Espressif forum to the
company and suggested they support FastLED ( why not? it's open source! )
and they told me to get lost.