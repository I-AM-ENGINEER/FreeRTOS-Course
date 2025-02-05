#include "system.h"
#include "usart.h"
#include "main.h"

/******************************** Config ************************************ */

#define UART1_RX_QUQUE_SIZE         (32)   

/******************************** Tasks **************************************/

void System::TasksFunctions::UART_Receiver( void* args ){
    HAL_UART_Receive_IT(&huart1, &System::uart_rx_byte_buffer, 1);
    while(1){
        char rcv_byte;
        xQueueReceive(System::QueueHandlers::UART_RX_Bytes, &rcv_byte, portMAX_DELAY);
        switch(rcv_byte) {
            case '1':
                HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
                break;
            case '2':
                HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
                break;
            case '3':
                HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
                break;
        }
    }
}

/******************************** Init ***************************************/

void SystemRun( void ){
    xTaskCreate(System::TasksFunctions::UART_Receiver, 
                "BTN checker",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::UART_Receiver
    );
    System::QueueHandlers::UART_RX_Bytes = xQueueCreate(UART1_RX_QUQUE_SIZE, sizeof(char));
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    vTaskStartScheduler();
}


/******************************** ISR Callbacks ******************************/

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart ){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_UART_Receive_IT(&huart1, &System::uart_rx_byte_buffer, 1);
    xQueueSendFromISR(System::QueueHandlers::UART_RX_Bytes, &System::uart_rx_byte_buffer, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
