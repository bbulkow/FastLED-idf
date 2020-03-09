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
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

#define NUM_LEDS 40
#define DATA_PIN 13 
#define BRIGHTNESS  80
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];


extern "C" {
  void app_main();
}


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
        leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
    }
    printf("show leds\n");
    FastLED.show();
    delay(400);
  };

};

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
			  leds[i] = colors[j];
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
	      leds[i] =   CRGB::Black;
	    }

	    // set the one LED to the right color
	    leds[pos] = colors_chase[led_color];
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
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
  // this is a good test because it uses the GPIO ports
  //FastLED.addLeds<APA102, 13, 15>(leds, NUM_LEDS);

  printf(" set max power\n");
  FastLED.setMaxPowerInVoltsAndMilliamps(5,2000);
  printf("create task for led blinking\n");
  xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
