/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "FastLED.h"
#include "FX.h"

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

//#define NUM_LEDS 512
#define NUM_LEDS 40
#define DATA_PIN_1 13 
#define DATA_PIN_2 18
#define BRIGHTNESS  80
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

extern "C" {
  void app_main();
}

/* test using the FX unit
**
*/

static void blinkWithFx(void *pvParameters) {

	uint16_t mode = FX_MODE_STATIC;

	WS2812FX ws2812fx;

	ws2812fx.init(NUM_LEDS, leds1, false); // type was configured before
	ws2812fx.setBrightness(255);
	ws2812fx.setMode(0 /*segid*/, mode);


	// microseconds
	uint64_t mode_change_time = esp_timer_get_time();

	while (true) {

		if ((mode_change_time + 10000000L) < esp_timer_get_time() ) {
			mode += 1;
			mode %= MODE_COUNT;
			mode_change_time = esp_timer_get_time();
			ws2812fx.setMode(0 /*segid*/, mode);
			printf(" changed mode to %d\n", mode);
		}

		ws2812fx.service();
		vTaskDelay(10 / portTICK_PERIOD_MS); /*10ms*/
	}
};



void ChangePalettePeriodically(){

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
    if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
    if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
    if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
    if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
    if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
    if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
    if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
    if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
  }

}

void blinkLeds_interesting(void *pvParameters){
  while(1){
  	printf("blink leds\n");
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds1[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        leds2[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
    }
    printf("show leds\n");
    FastLED.show();
    delay(400);
  };

};

// Going to use the ESP timer system to attempt to get a frame rate.
// According to the documentation, this is a fairly high priority,
// and one should attempt to do minimal work - such as dispatching a message to a queue.
// at first, let's try just blasting pixels on it.

// Target frames per second
#define FASTFADE_FPS 30

typedef struct {
  CHSV color;
} fastfade_t;

static void _fastfade_cb(void *param){

  fastfade_t *ff = (fastfade_t *)param;

  ff->color.hue++;

  if (ff->color.hue % 10 == 0) {
    printf("fast hsv fade h: %d s: %d v: %d\n",ff->color.hue,ff->color.s, ff->color.v);
  }

  fill_solid(leds1,NUM_LEDS,ff->color);
  fill_solid(leds2,NUM_LEDS,ff->color);

  FastLED.show();

};


static void fastfade(void *pvParameters){

  fastfade_t ff_t = {
    .color = CHSV(0/*hue*/,255/*sat*/,255/*value*/)
  };

  esp_timer_create_args_t timer_create_args = {
        .callback = _fastfade_cb,
        .arg = (void *) &ff_t,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "fastfade_timer"
    };

  esp_timer_handle_t timer_h;

  esp_timer_create(&timer_create_args, &timer_h);

  esp_timer_start_periodic(timer_h, 1000000L / FASTFADE_FPS );

  // suck- just trying this
  while(1){

      vTaskDelay(1000 / portTICK_PERIOD_MS);
  };

}

#define N_COLORS 17
CRGB colors[N_COLORS] = { 
  CRGB::AliceBlue,
  CRGB::ForestGreen,
  CRGB::Lavender,
  CRGB::MistyRose,
  CRGB::DarkOrchid,
  CRGB::DarkOrange,
  CRGB::Black,
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White,
  CRGB::Teal,
  CRGB::Violet,
  CRGB::Lime,
  CRGB::Chartreuse,
  CRGB::BlueViolet,
  CRGB::Aqua
};

void blinkLeds_simple(void *pvParameters){

 	while(1){

		for (int j=0;j<N_COLORS;j++) {
			printf("blink leds\n");

			for (int i=0;i<NUM_LEDS;i++) {
			  leds1[i] = colors[j];
        leds2[i] = colors[j];
			}
			FastLED.show();
			delay(1000);
		};
	}
};

#define N_COLORS_CHASE 7
CRGB colors_chase[N_COLORS_CHASE] = { 
  CRGB::AliceBlue,
  CRGB::Lavender,
  CRGB::DarkOrange,
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White,
};

void blinkLeds_chase(void *pvParameters) {
  int pos = 0;
  int led_color = 0;
  while(1){
  	printf("chase leds\n");

  		// do it the dumb way - blank the leds
	    for (int i=0;i<NUM_LEDS;i++) {
	      leds1[i] =   CRGB::Black;
        leds2[i] =   CRGB::Black;
	    }

	    // set the one LED to the right color
	    leds1[pos] = leds2[pos] = colors_chase[led_color];
	    pos = (pos + 1) % NUM_LEDS;

	    // use a new color
	    if (pos == 0) {
	    	led_color = (led_color + 1) % N_COLORS_CHASE ;
	    }

	    uint64_t start = esp_timer_get_time();
	    FastLED.show();
	    uint64_t end = esp_timer_get_time();
	    printf("Show Time: %" PRIu64 "\n",end-start);
	    delay(200);
	 };

}

void app_main() {
  printf(" entering app main, call add leds\n");
  // the WS2811 family uses the RMT driver
  FastLED.addLeds<LED_TYPE, DATA_PIN_1>(leds1, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2>(leds2, NUM_LEDS);

  // this is a good test because it uses the GPIO ports, these are 4 wire not 3 wire
  //FastLED.addLeds<APA102, 13, 15>(leds, NUM_LEDS);

  printf(" set max power\n");
  // I have a 2A power supply, although it's 12v
  FastLED.setMaxPowerInVoltsAndMilliamps(12,2000);

  // change the task below to one of the functions above to try different patterns
  printf("create task for led blinking\n");

  //xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&fastfade, "blinkLeds", 4000, NULL, 5, NULL, 0);
  //xTaskCreatePinnedToCore(&blinkWithFx, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
