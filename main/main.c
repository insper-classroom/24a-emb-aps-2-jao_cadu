
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
const uint BUTTON_E_PIN = 16;
const uint BUTTON_CTRL_PIN = 17;
const uint BUTTON_SHIFT_PIN = 18;


#define DEBOUNCE_TIME_MS 50  // Tempo de debounce em milissegundos

// Variável para armazenar a última vez que o botão foi pressionado
static uint32_t lastDebounceTimeButton = 0;
static uint32_t lastDebounceTimeButtonE = 0;
static uint32_t lastDebounceTimeButtonCtrl = 0;
static uint32_t lastDebounceTimeButtonShift = 0;

QueueHandle_t xQueueAdcData;
//QueueHandle_t xQueueCommands;

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




void btn_callback(uint gpio, uint32_t events) {
    
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Verifique se o tempo desde a última interrupção é maior que o tempo de debounce
    if ((currentTime - lastDebounceTimeButton) > DEBOUNCE_TIME_MS) {
        if (events == 0x4) {  // fall edge
            if (gpio == BUTTON_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 0, .val = 1};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_E_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 1, .val = 1};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_CTRL_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 2, .val = 1};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_CTRL_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 2, .val = 1};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }
        } else if (events == 0x8) {  // rise edge
            if (gpio == BUTTON_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 0, .val = 0};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_E_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 1, .val = 0};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_CTRL_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 2, .val = 0};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }if (gpio == BUTTON_SHIFT_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 3, .val = 0};
                xQueueSendToFront(xQueueAdcData, &data, portMAX_DELAY);
            }
        }
        // Atualize o último tempo de debounce processado
        lastDebounceTimeButton = currentTime;
        lastDebounceTimeButtonE = currentTime;
        lastDebounceTimeButtonCtrl = currentTime;
        lastDebounceTimeButtonShift = currentTime;
    }
}
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
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;
        uart_putc_raw(uart0, 3);  
        uart_putc_raw(uart0, data.axis);  
        uart_putc_raw(uart0, lsb);
        uart_putc_raw(uart0, msb);
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
}int main() {
    stdio_init_all();
    adc_init(); // Inicializa o ADC
    uart_init(uart0, 115200);  // Configura a UART0 para 115200 bps

    // Configura os pinos GPIO para o ADC
    adc_gpio_init(ADC_PIN_X1);
    adc_gpio_init(ADC_PIN_Y1);
    // adc_gpio_init(ADC_PIN_X2);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_init(BUTTON_E_PIN);
    gpio_set_dir(BUTTON_E_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_E_PIN);
    gpio_init(BUTTON_SHIFT_PIN);
    gpio_set_dir(BUTTON_SHIFT_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_SHIFT_PIN);
    gpio_init(BUTTON_CTRL_PIN);
    gpio_set_dir(BUTTON_CTRL_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_CTRL_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_E_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_SHIFT_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_CTRL_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    
    


    xQueueAdcData = xQueueCreate(10, sizeof(joystick_data_t));
    //xQueueCommands = xQueueCreate(10, sizeof(joystick_data_t));

    // Cria tarefas para cada eixo de cada joystick
    xTaskCreate(x1_task, "X1_Task", 256, NULL, 1, NULL);
    xTaskCreate(y1_task, "Y1_Task", 256, NULL, 1, NULL);
    // xTaskCreate(x2_task, "X2_Task", 256, NULL, 1, NULL);
    // xTaskCreate(y2_task, "Y2_Task", 256, NULL, 1, NULL);
    xTaskCreate(uart_task, "UART_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler(); // Inicia o escalonador

    while (true); // O loop principal não deve ser alcançado
    return 0;
}