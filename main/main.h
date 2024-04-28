#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>  // For uint32_t and other integer types

// Type definitions
typedef struct {
    int dici;   // Type of input device or data
    int axis;   // Axis of the joystick
    int val;    // Value from the joystick or button press
} joystick_data_t;

// Function declarations
void write_package(joystick_data_t data);
void hc06_task(void *params);
void uart_task(void *params);
void tremor_task(void *params);
void x1_task(void *params);
void y1_task(void *params);
void btn_callback(unsigned int gpio, uint32_t events);

#endif // MAIN_H
