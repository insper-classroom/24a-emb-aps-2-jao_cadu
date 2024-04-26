#include "main.h"

// Inicialização das bibliotecas
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hc06.h"

// Definições dos pinos
const uint ADC_PIN_X1 = 26;
const uint ADC_PIN_Y1 = 27;
const uint BUTTON_PIN = 15;
const uint BUTTON_E_PIN = 16;
const uint BUTTON_CTRL_PIN = 17;
const uint BUTTON_SHIFT_PIN = 18;
const uint BUTTON_A_PIN = 13;
const uint BUTTON_W_PIN = 12;
const uint BUTTON_S_PIN = 11;
const uint BUTTON_D_PIN = 10;
const uint BUTTON_ME_PIN = 8;
const int TREMER_PIN = 9;

#define DEBOUNCE_TIME_MS 50

// Declaração das queues e semáforo
QueueHandle_t xQueueAdcData;
QueueHandle_t xQueueButtonData;
SemaphoreHandle_t xSemaphoreTremor;

// Estrutura de dados para joystick
typedef struct {
    int dici;
    int axis;
    int val;
} joystick_data_t;
 

void write_package(joystick_data_t data) {

    //printf("%d /n",data.dici);

    if (data.dici == 1){
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;
        //printf("DENTRO DICI_1 /n");
        uart_putc_raw(HC06_UART_ID, 1);  
        uart_putc_raw(HC06_UART_ID, data.axis);  
        uart_putc_raw(HC06_UART_ID, lsb);
        uart_putc_raw(HC06_UART_ID, msb);
        uart_putc_raw(HC06_UART_ID, -1);
    }else if(data.dici == 2){
        uart_putc_raw(HC06_UART_ID, 2);  
        if(data.val == 1){
            uart_putc_raw(HC06_UART_ID, 1);  
        }else if(data.val == 2){
            uart_putc_raw(HC06_UART_ID, 2); 
        }else if(data.val == 3){
            uart_putc_raw(HC06_UART_ID, 3); 
        }else if(data.val == 4){
            uart_putc_raw(HC06_UART_ID, 4); 
        }
        uart_putc_raw(HC06_UART_ID, 0);  
        uart_putc_raw(HC06_UART_ID, 0);  
        uart_putc_raw(HC06_UART_ID, -1);
    }else if(data.dici == 3){
        int msb = data.val >> 8;
        int lsb = data.val & 0xFF;
        uart_putc_raw(HC06_UART_ID, 3);  
        uart_putc_raw(HC06_UART_ID, data.axis);  
        uart_putc_raw(HC06_UART_ID, lsb);
        uart_putc_raw(HC06_UART_ID, msb);
        uart_putc_raw(HC06_UART_ID, -1);
    }
}

void hc06_task(void *params) {

    uart_init(HC06_UART_ID, HC06_BAUD_RATE);
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
    hc06_init("APS2_JB", "1234");


    joystick_data_t data;
    while (1) {
        if (xQueueReceive(xQueueAdcData, &data, 1)) {
            write_package(data);
            vTaskDelay(10);
            write_package(data);
            //printf("%d,%d",data.axis, data.val);
        }
        if  (xQueueReceive(xQueueButtonData, &data, 1)) {
            write_package(data);
            //printf("%d,%d",data.axis, data.val);
        }
       
    }
}
// Tarefa UART revisada para usar write_package
void uart_task(void *params) {
    joystick_data_t data;
    while (1) {
        if (xQueueReceive(xQueueAdcData, &data, 1)) {
           
            write_package(data);
        }


    }
}

// Tarefas para o joystick 1
void x1_task(void *params) {
    joystick_data_t data = {.dici = 1, .axis = 0};
    while (1) {
        adc_select_input(0); // ADC Channel 0
        int adc_x = adc_read();

        if ((adc_x - 1900) / 12 > -70 && (adc_x - 1900) / 12 < 70) {
            data.val = 0;
        } else {
            data.val = ((adc_x - 1900) / 12)*(-1);  // Lê o valor ADC do eixo X
        }
        if(data.val !=0){
            xQueueSend(xQueueAdcData, &data, 1);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void y1_task(void *params) {
    joystick_data_t data = {.dici = 1, .axis = 1};
    while (1) {
        adc_select_input(1); // ADC Channel 1
        int adc_x = adc_read();

        if ((adc_x- 1900) / 12 > -70 && (adc_x - 1900) / 12 < 70) {
            data.val = 0;
        } else {
            data.val = ((adc_x - 1900) / 12)*(-1);  // Lê o valor ADC do eixo Y
        }
        if(data.val !=0){
            xQueueSend(xQueueAdcData, &data, 1);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void btn_callback(uint gpio, uint32_t events) {
    static uint32_t lastDebounceTimeButton = 0;
    
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Verifique se o tempo desde a última interrupção é maior que o tempo de debounce
    if ((currentTime - lastDebounceTimeButton) > DEBOUNCE_TIME_MS) {
        if (events == 0x4) {  // fall edge
            if (gpio == BUTTON_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 0, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_E_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 1, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_CTRL_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 2, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_SHIFT_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 3, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_A_PIN) {//A
                joystick_data_t data = {.dici = 3, .axis = 4, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_W_PIN) {//W
                joystick_data_t data = {.dici = 3, .axis = 5, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_S_PIN) {//S
                joystick_data_t data = {.dici = 3, .axis = 6, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_D_PIN) {//D
                joystick_data_t data = {.dici = 3, .axis = 7, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_ME_PIN) {//MOUSE
                xSemaphoreGiveFromISR(xSemaphoreTremor, NULL);
                joystick_data_t data = {.dici = 3, .axis = 8, .val = 1};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }
        } else if (events == 0x8) {  // rise edge
            if (gpio == BUTTON_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 0, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_E_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 1, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_CTRL_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 2, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_SHIFT_PIN) {
                joystick_data_t data = {.dici = 3, .axis = 3, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_A_PIN) {//A
                joystick_data_t data = {.dici = 3, .axis = 4, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_W_PIN) {//W
                joystick_data_t data = {.dici = 3, .axis = 5, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_S_PIN) {//S
                joystick_data_t data = {.dici = 3, .axis = 6, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_D_PIN) {//D
                joystick_data_t data = {.dici = 3, .axis = 7, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }if (gpio == BUTTON_ME_PIN) {//MOUSE
                joystick_data_t data = {.dici = 3, .axis = 8, .val = 0};
                xQueueSendToFront(xQueueButtonData, &data, 0);
            }
        }
        // Atualize o último tempo de debounce processado
        lastDebounceTimeButton = currentTime;
    }
}
// Função para formatar e enviar dados ou comandos via UART


void tremor_task(void *params) {
    while (1) {
        if (xSemaphoreTake(xSemaphoreTremor, portMAX_DELAY) == pdTRUE) {
            gpio_put(TREMER_PIN, 1);  // Ligar tremor
            vTaskDelay(pdMS_TO_TICKS(200)); // Tremor ligado por 200 ms
            gpio_put(TREMER_PIN, 0);  // Desligar tremor/* /*  */
        }
    }
}


int main() {
    stdio_init_all();
    adc_init(); // Inicializa o ADC */
    uart_init(HC06_UART_ID, 9600);  // Configura a UART0 para 115200 bps
    //printf("HELLO WORLD");

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
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_init(BUTTON_W_PIN);
    gpio_set_dir(BUTTON_W_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_W_PIN);
    gpio_init(BUTTON_S_PIN);
    gpio_set_dir(BUTTON_S_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_S_PIN);
    gpio_init(BUTTON_D_PIN);
    gpio_set_dir(BUTTON_D_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_D_PIN);
    gpio_init(BUTTON_ME_PIN);
    gpio_set_dir(BUTTON_ME_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_ME_PIN);

    
    gpio_init(TREMER_PIN);
    gpio_set_dir(TREMER_PIN, GPIO_OUT);

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
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);    
    gpio_set_irq_enabled_with_callback(BUTTON_W_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_S_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_D_PIN,
                                     GPIO_IRQ_EDGE_RISE | 
                                     GPIO_IRQ_EDGE_FALL, 
                                     true,
                                     &btn_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_ME_PIN,
                                   GPIO_IRQ_EDGE_RISE | 
                                   GPIO_IRQ_EDGE_FALL, 
                                   true,
                                   &btn_callback);

    xQueueAdcData = xQueueCreate(2, sizeof(joystick_data_t));
    xQueueButtonData = xQueueCreate(10, sizeof(joystick_data_t));

    xSemaphoreTremor = xSemaphoreCreateBinary();

    // Cria tarefas para cada eixo de cada joystick
    xTaskCreate(x1_task, "X1_Task", 256, NULL, 1, NULL);
    xTaskCreate(y1_task, "Y1_Task", 256, NULL, 1, NULL);
    xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);
    xTaskCreate(tremor_task, "Tremor_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler(); // Inicia o escalonador

    while (true); // O loop principal não deve ser alcançado
    return 0;
}
