
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

// Definições dos pinos para os joysticks
const uint ADC_PIN_X1 = 26; // GPIO 26, ADC Channel 0
const uint ADC_PIN_Y1 = 27; // GPIO 27, ADC Channel 1
const uint ADC_PIN_X2 = 28; // GPIO 28, ADC Channel 2
const uint ADC_PIN_Y2 = 29; // GPIO 29, ADC Channel 3

QueueHandle_t xQueueAdcData;
QueueHandle_t xQueueCommands;

typedef struct {
    int axis;  // 0 para X, 1 para Y
    int val;
    char command[10];  // Comando, se aplicável
} joystick_data_t;


// Tarefas para o joystick 1
void x1_task(void *params) {
    joystick_data_t data = {.axis = 0, .command = ""};
    while (1) {
        adc_select_input(0); // ADC Channel 0
        if ((adc_read() - 2047) / 8 > -30 && (adc_read() - 2047) / 8 < 30) {
            data.val = 0;
        } else {
            data.val = (adc_read() - 2047) / 8;  // Lê o valor ADC do eixo X
        }
        xQueueSend(xQueueAdcData, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void y1_task(void *params) {
    joystick_data_t data = {.axis = 1, .command = ""};
    while (1) {
        adc_select_input(1); // ADC Channel 1
        if ((adc_read() - 2047) / 8 > -30 && (adc_read() - 2047) / 8 < 30) {
            data.val = 0;
        } else {
            data.val = (adc_read() - 2047) / 8;  // Lê o valor ADC do eixo X
        }
        xQueueSend(xQueueAdcData, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefas para o joystick 2
void x2_task(void *params) {
    joystick_data_t data = {.axis = 0, .command = ""};
    while (1) {
        adc_select_input(2); // ADC Channel 2
        int x_val = (adc_read() - 2047) / 8;
        if (x_val < -30) {
            strcpy(data.command, "botao A");
        } else if (x_val > 30) {
            strcpy(data.command, "botao D");
        } else {
            strcpy(data.command, "");
        }
        xQueueSend(xQueueCommands, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void y2_task(void *params) {
    joystick_data_t data = {.axis = 1, .command = ""};
    while (1) {
        adc_select_input(3); // ADC Channel 3
        int y_val = (adc_read() - 2047) / 8;
        if (y_val < -30) {
            strcpy(data.command, "botao S");
        } else if (y_val > 30) {
            strcpy(data.command, "botao W");
        } else {
            strcpy(data.command, "");
        }
        xQueueSend(xQueueCommands, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// Função para formatar e enviar dados ou comandos via UART
void write_package(joystick_data_t data) {
    if (data.command[0] != '\0') {
        // Se há um comando, envie o comando
        //printf("Sending Command: Axis %d, Command %s\n", data.axis, data.command);
        uart_puts(uart0, "Command: ");
        uart_puts(uart0, data.command);
        uart_puts(uart0, "\n");
    } else {
        // Caso contrário, envie os dados ADC
        printf("Sending ADC Data (Não command): Axis %d, Value %d\n", data.axis, data.val);
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;
        uart_putc_raw(uart0, data.axis);  // Converte o índice do eixo para caracter
        uart_putc_raw(uart0, lsb);
        uart_putc_raw(uart0, msb);
        uart_putc_raw(uart0, '\n');
    }
}

// Tarefa UART revisada para usar write_package
void uart_task(void *params) {
    joystick_data_t data;
    while (1) {
        if (xQueueReceive(xQueueAdcData, &data, portMAX_DELAY)) {
            printf("Received ADC Data (Não command): Axis %d, Value %d\n", data.axis, data.val);
            write_package(data);
        }
        if (xQueueReceive(xQueueCommands, &data, portMAX_DELAY) && data.command[0] != '\0') {
            printf("Received Command: Axis %d, Command %s\n", data.axis, data.command);
            write_package(data);
        }
    }
}


int main() {
    stdio_init_all();
    adc_init(); // Inicializa o ADC
    uart_init(uart0, 115200);  // Configura a UART0 para 115200 bps

    // Configura os pinos GPIO para o ADC
    adc_gpio_init(ADC_PIN_X1);
    adc_gpio_init(ADC_PIN_Y1);
    adc_gpio_init(ADC_PIN_X2);
    adc_gpio_init(ADC_PIN_Y2);

    xQueueAdcData = xQueueCreate(10, sizeof(joystick_data_t));
    xQueueCommands = xQueueCreate(10, sizeof(joystick_data_t));

    // Cria tarefas para cada eixo de cada joystick
    xTaskCreate(x1_task, "X1_Task", 256, NULL, 1, NULL);
    xTaskCreate(y1_task, "Y1_Task", 256, NULL, 1, NULL);
    xTaskCreate(x2_task, "X2_Task", 256, NULL, 1, NULL);
    xTaskCreate(y2_task, "Y2_Task", 256, NULL, 1, NULL);
    xTaskCreate(uart_task, "UART_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler(); // Inicia o escalonador

    while (true); // O loop principal não deve ser alcançado
    return 0;
}

