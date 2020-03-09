# How to think about Threads in FastLED with ESP-IDF

I believe I understand how this is supposed to work.

## Note! 3 wire only

Everything below about RMT and parallel use likely flies out the
window with a 4-wire LED. No one has ported FastLED on ESP32 to use
hardware assiste for 4-wire LEDs, that's why you get that ugly
warning about no hardware assist.

## Parallel output, or multiple controller, mode

FastLED talks a bit about what they call 'multi channel'. This is
the very cool feature which allows parallel channels to all be blasted
at the same time, because you have hardware support.

The ESP32 has the RMT driver, which is a bit of hardware assist
which is used to blast out the waveform from a very small interrupt handler.

The examples in the Wiki don't match the way the code seems to be written
for this platform --- and it's a little weird.

For this platform, when you do a showLeds(), it keeps track of the number
of calls, and when you reach the same number of calls as the number of allocated
controllers, it actually spins up all of them together. I believe
this is intended to work because showLeds() will call each
of them in turn, so you use that.

I worry if you try to call the showLeds on individual controllers
and hope the right thing happens.

You'd prefer an interface called something like "flushLeds" which
acts on all the known controllers, or takes the list of controllers
you'd like to operate over.

I think this API is supposed to work that you bang all the pixel arrays,
then call showLeds() which hits all the controllers in turn, the last
of which actually kicks off the transfers for all the controllers.

## using the individual showLeds() functions

If you set up individual showLeds() functions on the different controller, the
code looks like the early ones will be very quick, and the final controller will 
do all the transfers.

I'd have to do some timing work and see if that's really happening --- but I'm at least 50% certain.

## Protecting the LED array, or using the external RMT driver

The API contract of showLEDs appears to be that showLeds blocks while
bytes are being pushed, which means its safe to molest the led array after
showLeds() finishes, thus setting you up for the next call.

Which is well and good, and allows you to do things like have a semaphore or 
signal that fires off when the frame is blasted.

There are two peculiarities with this concept.

If you are using the EXTERNAL_RMT, then each controller allocates a buffer 3x larger
than the number of pixels. It does a convert() on the pixel array into the big memory
buffer, which is the RMT buffer of signal transitions, and then doesn't molest your
pixel buffer.

While this a lot of memory, you've just achieved double-buffering. The actual pixel array
is clear to be modified right after that 'build' happens, and it would be super nice to 
know if that was true.

If you use the internal system, then the conversion happens in the interrupt routine.
The amount of work is the same ( roughly ), but most of it is happening in the interrupt
instead of out in "regular" time. Depending on your use, it might be better
to have the work going on in regular time as part of the double-buffer.

While you are sure that your Pixel arrays are safe after showPixels finishes, you've
covered a lot of time in the middle.

## How much CPU is really used?

If you 'do the math' on the number of pixels you can support, and you have 8 channels ( the number of RMT channels ),
you'll think you don't have much time to actually update your models. But, in fact, you do,
because the CPU is taking a fraction of its time feeding the RMT buffer, and a majority
of its time hanging on the semaphore waiting to be done.

The right thing to do, then, is to create another task to do whatever set of interpolation
you'd like to do.

If you measure the amount of time spent waiting on the Semaphore you'd be part of the way there,
but you should also measure the amount of CPU spent in the IRQ handler. Minus those out, 
and you'd learn how much CPU you've really got left.

## Rumination on double buffering

The use of the RMT buffer for external systems means you'd like to have some kind of control interface - like
a semaphore - on each individual pixel buffer. That would allow a higher level task to simply bang
away on the array and get held off when it was unsafe.

In the case of using the Internal RMT, the amount of time is almost the same as ShowPixels, so you
could easily have showPixels signal outside code through the mechanism of your choice.

I'm not aware, at the moment, of how you would measure the amount of CPU spent in the IRQ system,
which is the bulk of the time.

