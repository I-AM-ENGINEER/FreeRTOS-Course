#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#ifdef __cplusplus

namespace System{
    namespace TasksFunctions{
        void UART_Receiver( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t UART_Receiver;
    }
    namespace QueueHandlers{
        QueueHandle_t UART_RX_Bytes;
    }
    uint8_t uart_rx_byte_buffer;
}

extern "C" {
#endif

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart );
void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
