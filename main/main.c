
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

// Definições dos pinos para os joysticks
const uint ADC_PIN_X1 = 26; // GPIO 26, ADC Channel 0
const uint ADC_PIN_Y1 = 27; // GPIO 27, ADC Channel 1
// const uint ADC_PIN_X2 = 28; // GPIO 28, ADC Channel 2

const uint BUTTON_PIN = 15;

QueueHandle_t xQueueAdcData;
QueueHandle_t xQueueCommands;

typedef struct {
    int dici;  // Dicionario [1 -- joystick, 2 -- AWSD, 3 -- Botoes]
    int axis;  // 0 para X, 1 para Y
    int val;  // valor ou botao
} joystick_data_t;


// Tarefas para o joystick 1
void x1_task(void *params) {
    joystick_data_t data = {.dici = 1, .axis = 0};
    while (1) {
        adc_select_input(0); // ADC Channel 0
        if ((adc_read() - 2047) / 8 > -40 && (adc_read() - 2047) / 8 < 40) {
            data.val = 0;
        } else {
            data.val = (adc_read() - 2047) / 12;  // Lê o valor ADC do eixo X
        }
        xQueueSend(xQueueAdcData, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void y1_task(void *params) {
    joystick_data_t data = {.dici = 1, .axis = 1};
    while (1) {
        adc_select_input(1); // ADC Channel 1
        if ((adc_read() - 2047) / 8 > -40 && (adc_read() - 2047) / 8 < 40) {
            data.val = 0;
        } else {
            data.val = (adc_read() - 2047) / 12;  // Lê o valor ADC do eixo X
        }
        xQueueSend(xQueueAdcData, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefas para o joystick 2
// void x2_task(void *params) {
//     joystick_data_t data = {.dici = 2, .axis = 0};
//     while (1) {
//         adc_select_input(2); // ADC Channel 2
//         int x_val = (adc_read() - 2047) / 8;
//         if (x_val < -30) {
//             data.val = 1; //A
//         } else if (x_val > 30) {
//             data.val = 2; //D
//         } else {
//             data.val = 0;
//         }
//         xQueueSend(xQueueCommands, &data, portMAX_DELAY);
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// void y2_task(void *params) {
//     joystick_data_t data = {.dici = 2, .axis = 0};
//     while (1) {
//         adc_select_input(3); // ADC Channel 3
//         int y_val = (adc_read() - 2047) / 8;
//         if (y_val < -30) {
//             data.val = 3; //S
//         } else if (y_val > 30) {
//             data.val = 4; //W
//         } else {
//             data.val = 0; // Certifique-se de limpar o comando se estiver dentro do limiar
//         }
//         xQueueSend(xQueueCommands, &data, portMAX_DELAY);
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }



// void button_space_task(void *params) {
//     joystick_data_t data = {.dici = 2, .axis = -1, .val = 3};
//     bool buttonPressed = false;

//     while (1) {
//         if (gpio_get(BUTTON_PIN) == 0) {  // Se o botão estiver pressionado (leitura baixa)
//             if (!buttonPressed) {  // Verifica se o botão foi apenas pressionado
//                 buttonPressed = true;
//                 xQueueSend(xQueueAdcData, &data, portMAX_DELAY);
//             }
//         } else {
//             buttonPressed = false;  // Reseta o estado quando o botão é solto
//         }
//         vTaskDelay(pdMS_TO_TICKS(50));  // Debounce delay
//     }
// }


// Função para formatar e enviar dados ou comandos via UART
void write_package(joystick_data_t data) {

    //printf("%d /n",data.dici);

    if (data.dici == 1){
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;
        //printf("DENTRO DICI_1 /n");
        uart_putc_raw(uart0, 1);  
        uart_putc_raw(uart0, data.axis);  
        uart_putc_raw(uart0, lsb);
        uart_putc_raw(uart0, msb);
        uart_putc_raw(uart0, -1);
    }else if(data.dici == 2){
        uart_putc_raw(uart0, 2);  
        if(data.val == 1){
            uart_putc_raw(uart0, 1);  
        }else if(data.val == 2){
            uart_putc_raw(uart0, 2); 
        }else if(data.val == 3){
            uart_putc_raw(uart0, 3); 
        }else if(data.val == 4){
            uart_putc_raw(uart0, 4); 
        }
        uart_putc_raw(uart0, 0);  
        uart_putc_raw(uart0, 0);  
        uart_putc_raw(uart0, -1);
    }else if(data.dici == 3){
        uart_putc_raw(uart0, 3);  
        uart_putc_raw(uart0, 0);  
        uart_putc_raw(uart0, 0);  
        uart_putc_raw(uart0, 0);  
        uart_putc_raw(uart0, -1);
    }
}

// Tarefa UART revisada para usar write_package
void uart_task(void *params) {
    joystick_data_t data;
    while (1) {
        if (xQueueReceive(xQueueAdcData, &data, portMAX_DELAY)) {
            write_package(data);
        }
        // if (xQueueReceive(xQueueCommands, &data, portMAX_DELAY)) {
        //     write_package(data);
        // }
    }
}



int main() {
    stdio_init_all();
    adc_init(); // Inicializa o ADC
    uart_init(uart0, 115200);  // Configura a UART0 para 115200 bps

    // Configura os pinos GPIO para o ADC
    adc_gpio_init(ADC_PIN_X1);
    adc_gpio_init(ADC_PIN_Y1);
    // adc_gpio_init(ADC_PIN_X2);
    // gpio_init(BUTTON_PIN);
    // gpio_set_dir(BUTTON_PIN, GPIO_IN);
    // gpio_pull_up(BUTTON_PIN);


    xQueueAdcData = xQueueCreate(10, sizeof(joystick_data_t));
    xQueueCommands = xQueueCreate(10, sizeof(joystick_data_t));

    // Cria tarefas para cada eixo de cada joystick
    xTaskCreate(x1_task, "X1_Task", 256, NULL, 1, NULL);
    xTaskCreate(y1_task, "Y1_Task", 256, NULL, 1, NULL);
    // xTaskCreate(x2_task, "X2_Task", 256, NULL, 1, NULL);
    // xTaskCreate(y2_task, "Y2_Task", 256, NULL, 1, NULL);
    // xTaskCreate(button_space_task, "button_space_task", 256, NULL, 1, NULL);
    xTaskCreate(uart_task, "UART_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler(); // Inicia o escalonador

    while (true); // O loop principal não deve ser alcançado
    return 0;
}

