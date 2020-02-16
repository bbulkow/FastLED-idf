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

# Use of ESP32 hardware

The ESP32 has an interesting module, called RMT. It's a module that's
meant to make arbitrary waveforms on pins, without having to bang each pin at
the right time. While it was made for IR Remote Control devices, 
it works great for LEDs, and appears to be better than the PWM hardware.

There are 8 channels, which tends to match well with the the desires of most
people for LED control. 8 channels is basically still a ton of LEDs, even
if the FastLED ESP32 module is even fancier and multiplexes the use of these
channels.

The FastLED ESP32 RMT use has two modes: one which uses the "driver", and
one which doesn't, and claims to be more efficient due to when it's converting
between LED RGB and not.

Whether you can use the "direct" mode or not depends on whether you have other
users of the RMT driver within ESP-IDF. 

Essentially, if you have the Driver turned on, you shouldn't use the direct mode,
and if you want to use the direct mode, you should turn off the driver.

TODO: insert specific information about turning these things on and off in `menuconfig`

# A bit about esp-idf

ESP-IDF, in its new 4.0 incarnation, has moved entirely to a cmake infrastructure.
It creates a couple of key files at every level: `CMakeLists.txt` , `Kconfig`. 

The `CMakeLists.txt` is where elements like which directories have source and includes live.
Essentially, these define your projects.

The `Kconfig` files allow you to create variables that show up in MenuConfig. When you're building
a new project, what you do is run `idf.py menuconfig` and that allows you to set key variables,
like whether you want to bit-bang or use optimized hardware, which cores to run on, etc.

One element of esp-idf that took a while to cotton onto is that it doesn't build a single binary
then you build your program and link against it. Since there are so many interdependancies -
it's an RTOS, right - what really happens is when you build your code, you build everything
in the standard set too. This means compiles are really long because you're
rebuilding the entire system every time.

There's no way to remove components you're not using. Don't want to compile in HTTPS server? Tough.
The menuconfig system allows you to not run it, but you can't not compile it without
going in and doing surgery.

# History 

## The other FastLED-idf

It appears that set of code was an earlier version of FastLED, 
and probably only still works with ESP-IDF 3.x. There was a major
update to ESP-IDF, a new build system, lots of submodule updates,
cleaned up headers and names, which seem to be all for the better -
but with lots of breaking changes.

Thus, updating both, and using eskrab's version as a template, 
I attempt to have a running version of FastLED with the ESP-IDF
4.0 development environment

## ESP-IDF already has a LED library

Yes, I saw it once, I'm having trouble finding it now. It does
really cool timing tricks, but it's not got all the fancy-fancy
of the FastLED library, which has a better designed and time tested 
programming interface. I reached out on the Espressif forum to the
company and suggested they support FastLED ( why not? it's open source! )
and they told me to get lost.

# Updating

This package basically depends on three packages: two that you pull in,
and the fact that you've got to use the version that works with your intended version
of esp-idf. Those two packages are FastLED, and also arduino-esp32, since FastLED is
using the arduino interfaces.

It would have been possible to just have at the FastLED code and nuke the portions
that are Arduino-ish, however, that removes the obvious benefit of eventually
being able to update FastLED.

As a starting point, then I've taken the choice of keeping the libraries as unmodified
as possible.

## FastLED

Drop this into the FastLED-idf directory. Now, it might have been cleaner to create a subdirectory
with nothing but the FastLED-sourced code, this could be an organizational change the future.

## Arduino-esp32

You need a few of the HAL files. Please update these in the subdirectory hal.

# Gotchas and Todos

## ESP32 define

I banged my head for a few hours on how to define ESP32, which is needed by FastLED to choose
its platform. This should be doable in the CMakeLists.txt, but when I followed the instructions,
I got an error about the command not being scriptable. Thus, to move things along, I define-ed
at the top of the FastLED.h file.

Someone please fix that

## lots of make threads makes dev hard

It seems `idf.py build` uses cores+2. That means when you're actually building your component, you're in line with
all the other esp idf components, and you'll see a lot of stuff compile then finally what you want.

The best way is to do a few builds - which will all fail, and the other components without errors ( the ones in esp-idf )
will finally get built, then you'll be left with only your errors.

Building without `-j` parallelism would be nice, but I haven't found a way to do that. And, it would be slow.

