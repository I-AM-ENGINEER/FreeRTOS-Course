#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "usart.h"

#ifdef __cplusplus

class UART_STM32{
public:
    UART_STM32( UART_HandleTypeDef* huart ) :
          _huart(huart)
    {
        _send_mutex = xSemaphoreCreateMutex();
    }

    size_t Send( const uint8_t* buff, size_t size, uint32_t timeout ){
        size_t cnt = 0;
        HAL_StatusTypeDef res;
        TickType_t timestamp1 = xTaskGetTickCount();
        xSemaphoreTake(_send_mutex, pdMS_TO_TICKS(timeout));
        uint32_t semaphore_take_time = pdTICKS_TO_MS(xTaskGetTickCount() - timestamp1);
        if(semaphore_take_time > timeout){
            goto timeout_event;
        }
        timeout -= semaphore_take_time;
        res = HAL_UART_Transmit(_huart, buff, static_cast<uint16_t>(size), timeout);
        if(res != HAL_OK){
            goto timeout_event;
        }
        cnt = size;
timeout_event:
        xSemaphoreGive(_send_mutex);
        return cnt;
    }
private:
    QueueHandle_t _send_mutex;
    UART_HandleTypeDef* _huart;
};


namespace System{
    namespace TasksFunctions{
        void UART_Sender1( void* args );
        void UART_Sender2( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t UART_Sender1;
        TaskHandle_t UART_Sender2;
    }
    UART_STM32 uart1(&huart1);
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
