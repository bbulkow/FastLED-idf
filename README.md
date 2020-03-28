# FastLED-idf

# TL;DR

This port of FastLED 3.3 runs under the 4.0 ESP-IDF development environment. Enjoy.

Note you must use the ESP-IDF environment, and the ESP-IDF build system. That's how the paths and whatnot are created.

Here is the link to the ESP-IDF getting started guide.
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

Pull requests welcome.

# Why we need FastLED-idf

The ESP32 is a pretty great SOC package. It has two
cores, 240Mhz speed, a meg of DRAM ( sorta ), low power mode, wifi and bluetooth, and can be had in pre-built 
modules at prices between $24 ( Adafruit, Sparkfun ), $10 (Espressif-manufactured boards through 
Mauser and Digikey), or 'knock off' boards for $4 from AliExpress as of 2020.

If you're going to program this board, you might use Arduino.
Although I love the concept of Arduino - and the amazing amount
of libraries - the reality is the Arduino IDE is a mess, and the compile
environment is "funky", that is, it's not really C - about my first
minute in the IDE, I managed to write a perfectly valid C preprocessor
directive that's illegal in Arduino.

Enter ESP-IDF, which is Espressif's RTOS for this platform. It's based on FreeRTOS,
but they had to fork it for multiple cores. It's based on FreeRTOS 9,
but has some of the work done later backported so it has a few of the FreeRTOS 10 functions, apparently.
Development seems vibrant, and they use the stock GCC 8.0 compiler with no strange overlays.

There are a TON of useful modules included with ESP-IDF. Notably,
nghttp server, mdns, https servers, websockets, json, mqtt, etc etc.

What I tend to do with embedded systems is blink LEDs! Sure, there's other
fun stuff, but blinking LEDs is pretty good. The included `ledc` module in ESP-IDF is only for
changing the duty cycle and brightness, it doesn't control color-controlled LEDs like the WS8211.

Thus, we need FastLED

# TL;DR about this repo

As with any ESP-IDF project, there is a sdkconfig file. It contains things that might
or might not be correct for your ESP32. I've checked in a version that runs at 240Mhz,
runs both cores, uses 40Mhz 4MB DIO Flash, auto-selects the frequency of the clock, and
only runs on Rev1 hardware, and has turned off things like memory poisoning.

I've read scary stuff about Rev0 and GPIO. You have to insert a number of NOP statements
between bangs of the pins. If you're using that version, you might want to look carefully
into the issue.

# Use of ESP32 hardware for 3 wire LEDs

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
between LED RGB and not. I have not tested whether using the ESP driver
works.

Whether you can use the "direct" mode or not depends on whether you have other
users of the RMT driver within ESP-IDF. 

Essentially, if you have the Driver turned on, you shouldn't use the direct mode,
and if you want to use the direct mode, you should turn off the driver.

No extra commands in `menuconfig` seem necessary.

# Four wire LEDs ( APA102 and similar )

Interestingly, four wire LEDs can't use the RMT interface, because
the clock and data lines have to be controled together ( duh ),
and the RMT interface doesn't do that. What does do that is the SPI
interface, and I don't think I've wired that up.

There are two hardware SPI ports in the system, so that should be
able to be enabled. I haven't tried that.

Since hardware banging is used ( that's the big ugly warning ),
these LEDs are very likely far slower, and probably do not respond to parallelism
the same way.

I have pulled in enough of the HAL for the APA102 code to compile,
but I don't know if it works, since I don't have any four-wire LEDs
around. Since it's very much a different code path, I'm not going to make
promises until someone tries it.

# async mode

The right way to use a system like this is with some form of async mode, where the CPU
is loading and managing the RMT buffer(s), but then goes off to do other works while that's
happening. This would allow much better multi-channel work, because the CPU could fill 
one channel, then go off and fill the next channel, etc. For that, you'd have to use
the LastLED async interfaces.

It turns out this all works just peachy, it's just that the FastLED interface is a little
peculiar. If you see the THREADING document in this directory, you'll
see that this port is enabling multi-channel mode when using 3-wire LEDs,
which means you change all the pixels, and when you call FastLED.show(), they'll
all bang on the RMT hardware, and use very little main CPU.

It would be nicer if FastLED had some sort of Async mode, but that's not
really the Arduino way, and this code is meant for arduino. Arduino doesn't
have threads of control or message queues or anything like that.

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

Not really. There is an included 'ledc' library, which
simply changes the duty cycle on a pin using the RMT interface.
It doesn't do pixel color control. It can be an example of using
the RMT system, that's it.

There is an example of LED control, using the RMT interface directly.
If you just want to it like that, without all the cool stuff in FastLED,
be everyone's guest!

I did reach out to Espressif. I think they should include or have a FastLED port.
In their forums, they said they don't intend to do anything like that.

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

Drop this into the components/FastLED-idf directory. Now, it might have been cleaner to create a subdirectory
with nothing but the FastLED-source code, this could be an organizational change the future.

## Arduino-esp32

You need a few of the HAL files. Please update these in the subdirectory hal. Don't forget to close
the pod bay doors.

# Timing and speed

On an ESP32 running at 240Mhz, I was able to time 40 pixels at 1.2 milliseconds. I timed showLeds()
at 3.0 milliseconds at 100 LEDs.

However, if you use more than one controller ( see the THREADING document ),
three different controllers each will do 100 LEDs in 3 milliseconds. I have tested
this with 1, 3, and 6 controllers. They all take the same amount of time.

If 30 milliseconds makes 30fps, then you should be able to do 1000 pixels on one of the two cores, 
and still, potentially, have the ability to do some extra work, because you've got the other CPU.

I have not determined if this is using the RMT interface, which would mean we could 
use an async internal interface. The timing, however, is very solid.

# Use notes

What I like about using FreeRTOS and a more interesting development environment is
you should be able to use more of the CPU for other things. Essentially, you should
be able to have the RMT system feeding itself, which then allows the main CPU
to be updating the arrays of colors, and multiple channels to be doing the right
thing all at the same time.

However, really being multi-core would mean having a locking semantic around the color
array, or double buffering. FastLED doesn't seem to really think that way,
rightfully so.

# Liscening

FastLED is MIT license.

I intend my portions to be MIT license. AKA the don't sue me license.

However, the Espressif HAL code is LGPL. Mostly, these are used as headers,
not as code itself. There's very little of value there. If LGPL bothers
you, I would propose a quick rewrite of those files, and submit a pull request
that would be under MIT license.

I am honestly not sure what happens to LGPL in this case. It's a component in an
embedded system, which is morally a library, but it is clearly very statically
linked.

I don't intend to make any money off this, don't charge people, and do not intend
the use for commercial art projects, so the use is safe for me. But don't say
I didn't warn you.

# Gotchas and Todos

## ESP32 define

I banged my head for a few hours on how to define ESP32, which is needed by FastLED to choose
its platform. This should be doable in the CMakeLists.txt, but when I followed the instructions,
I got an error about the command not being scriptable. Thus, to move things along, I define-ed
at the top of the FastLED.h file.

Someone please fix that!

## lots of make threads makes dev hard

This is just a whine about esp-idf.

It seems `idf.py build` uses cores+2. That means when you're actually building your component, 
you're compiling all the other esp idf components, and you'll see a lot of stuff compile 
then finally what you want.

The best way is to do a few builds - which will all fail, and the other components 
without errors ( the ones in esp-idf )
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

## GPIO defined not found - get the hal

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
```
WRITE_PERI_REG(GPIO_OUT_W1TS_REG, 1 << GPIO_HANDSHAKE)
```

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
colorutils.h 
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

In the upstream code of FastLED, there is an introduction of a void * cast in these two places.
That's the other way of doing it, and the code does "promise" that the classes in question
can be memcpy'd. If someone re-ports the code, they could fix this.

## Don't use C

Had a main.c , and FastLED.h includes nothing but C++. Therefore, all the source files
that include FastLED have to be using the C++ compiler, and ESP-IDF has the standard rules
for using C for C and CPP for CPP because it's not like they are subsets or something.

## CXX_STUFF

There is some really scary code about guards. It is defined if ESP32, 
and seems to override the way the compiler executes a certain kind of guard.
I've turned that off, because I think we should probably trust esp-idf to do the
right and best thing for that

## pulled in too much of the hal

In order to get 4-wire LEDs compiling, it's necessary to pull in the HAL gpio.c . 
Just to get one lousy function - pinMode. I probably should
have simply had the function call gpio_set_direction(), which is the esp_idf
function, directly. If you have a 4-wire system, I left a breadcrumb in the
fastpin_esp32 directory. If the alternate code works, then you can take the
gpio.c out of the hal compile.

## message about no hardware SPI pins defined

It's true! There are no hardware SPI pins defined. SPI is used for 4-wire LEDs, where
you have to synchronize the clock and data.

If you have 3-wire LEDs, you'll be using the RMT system, which is super fast anyway.

The upstream code doesn't seem to use SPI on ESP32 either, so that's just a big hairy
todo, and don't worry overmuch.
