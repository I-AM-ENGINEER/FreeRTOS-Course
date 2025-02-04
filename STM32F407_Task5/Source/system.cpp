#include "system.h"
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "adc.h"
#include "usart.h"

/******************************** Config ************************************ */

#define QUEUE_MSG_LEN                         (100)
#define ADC_READ_PERIOD_MS                    (500)
#define QUEUE_HANDLER_TASK_MAX_TIMEOUT_MS     (1000)
#define UART1_PRINTF_BUFFER_SIZE              (64)
#define BTN_JITTER_MS                         (20)
#define BTN_PULL_RATE_HZ                      (50)

/******************************** Internal calcs **************************** */

#define BTN_PULL_RATE_PERIOD_MS (1000/BTN_PULL_RATE_HZ)

/******************************** Common functions ****************************/

// I dont like redefine default printf to UART, only RTT
void printf_uart1(const char *format, ...) {
    char buffer[UART1_PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
    }
}

bool IsButtonPressed( void ){
    return HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET;
}

/******************************** Tasks **************************************/

void System::TasksFunctions::ADC_Reader( void* arg ){
    TickType_t timestamp = xTaskGetTickCount();
    while(1){
        HAL_ADC_Start(&hadc1);
        while (!LL_ADC_IsActiveFlag_EOCS(ADC1)) {
            vTaskDelay(1);
        }
        uint16_t adc_val = (uint16_t)HAL_ADC_GetValue(&hadc1);
        queue_msg_t msg = {
            .id = MSG_ID_ADC,
            .val = adc_val,
        };
        xQueueSend(System::Queues::msgs, &msg, portMAX_DELAY);
        xTaskDelayUntil(&timestamp, pdTICKS_TO_MS(ADC_READ_PERIOD_MS));
    }
}

void System::TasksFunctions::QueueHandler( void* args ){
    while(1){
        queue_msg_t msg;
        BaseType_t res = xQueueReceive(System::Queues::msgs, &msg, QUEUE_HANDLER_TASK_MAX_TIMEOUT_MS);
        if(res == pdFALSE){
            printf_uart1("ERR: Queue timeout\n");
            continue;
        }
        switch(msg.id){
            case MSG_ID_KEYBOARD:
                printf_uart1("BTN pressed\n");
                HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
                break;
            case MSG_ID_ADC:
                printf_uart1("ADC PA0 value: %hu\n", msg.val);
                break;
            default: break;
        }
    }
}

void System::TasksFunctions::BTN_Checker( void* args ){
    bool old_btn_state = false;
    queue_msg_t msg = {
        .id = MSG_ID_KEYBOARD,
        .val = 0,
    };
    while(1){
        eTaskState blink_task_state;
        bool btn_state = IsButtonPressed();
        // If button pressed event
        if((old_btn_state != false) || (btn_state != true)){
            goto no_action; // If no press event
        }
        vTaskDelay(pdMS_TO_TICKS(BTN_JITTER_MS));
        if(IsButtonPressed() == false){
            goto no_action; // If button jitter no action
        }

        xQueueSend(System::Queues::msgs, &msg, portMAX_DELAY);
no_action:
        old_btn_state = btn_state;
        vTaskDelay(pdMS_TO_TICKS(BTN_PULL_RATE_PERIOD_MS));
    }
}

/******************************** Init ***************************************/

void SystemRun( void ){
  xTaskCreate(System::TasksFunctions::QueueHandler,
              "QueueHdl",
              configMINIMAL_STACK_SIZE,
              NULL,
              0,
              &System::TasksHandlers::QueueHandler
  );
  xTaskCreate(System::TasksFunctions::ADC_Reader,
              "ADC reader",
              configMINIMAL_STACK_SIZE,
              NULL,
              1,
              &System::TasksHandlers::ADC_Reader
  );
  xTaskCreate(System::TasksFunctions::BTN_Checker, 
              "BTN checker",
              configMINIMAL_STACK_SIZE,
              NULL,
              2,
              &System::TasksHandlers::BTN_Checker
  );
  
  System::Queues::msgs = xQueueCreate(QUEUE_MSG_LEN, sizeof(queue_msg_t));
  vTaskStartScheduler();
}
