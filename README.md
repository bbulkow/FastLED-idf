# FastLED-idf

# TL;DR

This port of FastLED 3.3 runs under the ESP-IDF development environment. Enjoy.

# Why we need FastLED-idf

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

No extra commands in `menuconfig` seem necessary.

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

It appears that set of code was an earlier version of FastLED, and probably only still 
works with ESP-IDF 3.x. There was a major update to ESP-IDF, a new build system, 
lots of submodule updates, cleaned up headers and names, which seem to be all for the better -
but with lots of breaking changes.

Thus, updating both, and using eskrab's version as a template, I attempt to have a 
running version of FastLED with the ESP-IDF 4.0 development environment

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
with nothing but the FastLED-source code, this could be an organizational change the future.

## Arduino-esp32

You need a few of the HAL files. Please update these in the subdirectory hal.

# Gotchas and Todos

## ESP32 define

I banged my head for a few hours on how to define ESP32, which is needed by FastLED to choose
its platform. This should be doable in the CMakeLists.txt, but when I followed the instructions,
I got an error about the command not being scriptable. Thus, to move things along, I define-ed
at the top of the FastLED.h file.

Someone please fix that!

## lots of make threads makes dev hard

It seems `idf.py build` uses cores+2. That means when you're actually building your component, you're in line with
all the other esp idf components, and you'll see a lot of stuff compile then finally what you want.

The best way is to do a few builds - which will all fail, and the other components without errors ( the ones in esp-idf )
will finally get built, then you'll be left with only your errors.

Building without `-j` parallelism would be nice, but I haven't found a way to do that. And, it would be slow.

## micros

Defined in arduino, maps to esp_timer_get_time() . There is an implementation of this in esp32-hal-misc.c, but no definition, because you are meant to supply a function that is defined by Arduino.

I have defined these functions in esp32-hal.h out of a lack of other places to put them.
And, wouldn't it be better to use defines for these instead of functions???

## port access for GPIO banging

The FastLED code is fast because it doesn't call digital read and digital write, it instead 
grabs the ports and ors directly into them.
This is done with `digitalPinToBitMask` and `digitalPinToPort`. Once you have the port, 
and you have the mask, bang, you can go to town.

The esp-idf code supports the usual set of function calls to bang bits, and they include if 
statements, math, and a huge check
to see if your pin number is right, inside every freaking inner loop. 
No wonder why a sane person would circumvent it.
However, the ESP32 is also running at 240Mhz instead of 16Mhz, so performance optimizations
will certainly matter less.

Looking at the eshkrab FastLED port, it appears that person just pulled in most of the
registers, and has run with it.

HOWEVER, what's more interesting is the more recent FastLED code has an implementation of everything
correctly under platforms///fastpin_esp32.h. There's a template there with all the right mojo.
In a q-and-a section, it says that the PIN class is unused these days, and only FastPin is used.
FastPin still uses 'digitalPinToPort', but has a #define in case those functions don't exist.

The following github issue is instructive. https://github.com/FastLED/FastLED/issues/766

It in fact says that the FASTLED_NO_PINMAP is precisely to be used in ports where there is no Arduino.
That's me! So let's go set that and move along to figuring out how to get the FastPins working.

## GPIO defined not found

Best current guess. There is an arduino add-only library called "Arduino_GPIO", which has the same
basic structure, and that's what's being used to gain access to the core register pointers and such.

Essentially, this has to be re-written, because GPIO in that way doesn't exist.

A few words about bitbanging here.

There appears to be 4 32-bit values. They are w1tc ( clear ) and w1ts ( set ). When you want
to write a 1, you set the correct values in w1ts, and when you want to clear, you set 1 to the
values you want to clear in w1tc. Since there are 40 pins, there are two pairs of these.

Explained here: https://esp32.com/viewtopic.php?t=1987 . And noted that perhaps you can only
set one value at a time, and it makes the system atomic, where if you try to read, mask, write
in a RTOS / multicore case, you'll often hurt yourself. No taking spinlocks in this case, which
is great.

There are also two "out" values. Oddly, there are both the GPIO.out, and GPIO.out.value. Unclear why.
Reading the code naively, it looks like almost a bug. How could it be that the high pins
have such a different name?

Another intereting post on the topic: https://www.esp32.com/viewtopic.php?t=1595 . This points
to raw speeds being in the 4mhz / 10mhz range, and says "use the RMT interface". It also
has questions about whether you need to or or set. The consensus seems to be that you don't need
to do that, and you shoudn't need to disable interrupts either, because these actions are atomic.

Now, let's figure out the best way to adapt those to ESP-IDF.

https://docs.espressif.com/projects/esp-idf/en/v4.0/api-reference/peripherals/gpio.html

IT looks like there are defines for these values in ESP-IDF, so you really just need to 
find the places these are defined and change the names to GPIO_OUT_W1TS_REG , and GPIO_OUT_REG
etc etc

soc/gpio_reg.h --- examples/peripherals/spi_slave
or maybe just driver/gpio.h?
WRITE_PERI_REG(GPIO_OUT_W1TS_REG, 1 << GPIO_HANDSHAKE)

Looks like if you grab "gpio.h", it'll include what you need.
For full information, it was bugging me where this structure is. It's in:
soc/esp32/include/soc/gpio_struct.h . Which I also think is pulled in
with gpio.h, or more correctly driver/gpio.h as below.

Hold up! It looks like `driver/gpio.c` uses the same exact GPIO. name. Is this a namespace or something?
Should I just start including the same files gpio.c does?

```
#include <esp_types.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/xtensa_api.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "soc/soc.h"
#include "soc/gpio_periph.h"
#include "esp_log.h"
#include "esp_ipc.h"
```

Let's start with:

```
#include <esp_types.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/xtensa_api.h"
#include "driver/gpio.h"
#include "soc/gpio_periph.h"
```

Yay! 

Note: what's up with registering the driver? I should, right? Make sure that's done in
menuconfig? Doen't seem to be. There are no settings anywhere in the menuconfig regarding
gpio. Should probably look at the examples to see if I have to enable the ISR or something.

## using RMT vs I2S

Note to self: There is a define in the platforms area that lets a person choose.

## GCC 8 memcpy and memmove into complex structures

```
/mnt/c/Users/bbulk/dev/esp/FastLED-idf/components/FastLED-idf/colorutils.h:455:69: error: 'void* memmove(void*, const void*, size_t)' writing to an object of type 'struct CHSV' with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Werror=class-memaccess]
         memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
```

Files involved are:
```
FastLED.h
bitswap.h
controller.cpp
colorutils.h 455
```

( Note: bitswap.cpp is rather inscrutable, since it points to a page that no longer exists. It would
be really nice to know what it is transposing, or shifting, AKA, what the actual function is intended
to do, since the code itself is not readable without a roadmap. )

The offending code is here:
```
    CHSVPalette16( const CHSVPalette16& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
```

and I think is an unnessary and unsafe optimization. It would be shocking on this processor, with GCC8+,
that the memmove8 optimization is sane. This is a straight-up initialization, and the code should probably
be simplified to do the simple thing.

This particular warning can be removed in a single file through the following pattern:
```
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
```

https://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code

After looking a bit, it doesn't seem unreasonable to remove all the memmoves and turn them into
loops. It's been done in other places of the code. Otherwise, one could say "IF GCC8" or something
similar, because I bet it really does matter on smaller systems. Maybe even on this one.

There is a menuconfig to turn off new warnings introduced from GCC6 to GCC8.

The other thing is to cast these to void, but ewww. Anyone who uses C++ on an embedded system
should go whole hog.

## Don't use C

Had a main.c , and FastLED.h includes nothing but C++. Therefore, all the source files
that include FastLED have to be using the C++ compiler, and ESP-IDF has the standard rules
for using C for C and CPP for CPP because it's not like they are subsets or something.

## message about no hardware SPI pins defined

Let's track down whether they are using or are not? And what about the RMI stuff?
