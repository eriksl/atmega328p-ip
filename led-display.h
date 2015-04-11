#ifndef led_display_h
#define led_display_h

#include <stdint.h>

void	display_brightness(uint8_t level);
uint8_t	display_show(const uint8_t *text);

#endif
