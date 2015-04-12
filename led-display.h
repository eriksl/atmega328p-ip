#ifndef led_display_h
#define led_display_h

#include <stdint.h>

uint8_t	led_display_brightness(uint8_t level);
void	led_display_clear(void);
void	led_display_show(const uint8_t *string);
void	led_display_show_chunk(uint8_t length, const uint8_t *string);
void	led_display_update(void);

#endif
