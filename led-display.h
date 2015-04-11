#ifndef led_display_h
#define led_display_h

#include "application.h"

#include <stdint.h>

extern uint8_t display_string[application_num_args - 1][5];

void	display_brightness(uint8_t level);
uint8_t	display_show(const uint8_t *text);

#endif
