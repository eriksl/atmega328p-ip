#ifndef application_uart_h
#define application_uart_h

#include "application.h"

#include <stdint.h>

uint8_t application_function_uart_baud(application_parameters_t);
uint8_t application_function_uart_transmit(application_parameters_t);
uint8_t application_function_uart_receive(application_parameters_t);

#endif
