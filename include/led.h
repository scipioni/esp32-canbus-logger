#pragma once

//const int led1 = LED_BUILTIN; // Pin of the LED
#define LED_PIN LED_BUILTIN
#define BLINK_FAST 300
#define BLINK_SLOW 900

extern int led_interval;

void led_setup();