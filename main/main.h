#ifndef MAIN_H
#define MAIN_H

// Inclua todas as bibliotecas necessárias aqui

// Declaração de funções
void write_package(joystick_data_t data);
void hc06_task(void *params);
void uart_task(void *params);
void tremor_task(void *params);
void x1_task(void *params);
void y1_task(void *params);
void btn_callback(uint gpio, uint32_t events);

#endif // MAIN_H
