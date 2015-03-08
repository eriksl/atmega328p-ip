#ifndef application_vfd_h
#define application_vfd_h

#include "application.h"

#include <stdint.h>

void application_init_vfd(void);
uint8_t application_function_dbr(application_parameters_t ap);
uint8_t application_function_dclr(application_parameters_t ap);
uint8_t application_function_dshow(application_parameters_t ap);
uint8_t application_function_dhshw(application_parameters_t ap);

#endif
