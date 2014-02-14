#include <stdint.h>

#include "enc.h"

#define DEBUG(s) static __attribute__((unused)) uint16_t s ## _; s ## _ = read_register(s);

void debug(void);
