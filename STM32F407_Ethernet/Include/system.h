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
        void ETH_State( void* args );
        void ETH_Sender( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t ETH_State;
        TaskHandle_t ETH_Sender;
    }
    UART_STM32 uart1(&huart1);
}

extern "C" {
#endif

//uint32_t HAL_GetTick(void);
void HAL_ETH_RxAllocateCallback(uint8_t** buff);
void HAL_ETH_RxLinkCallback(void** pStart, void** pEnd, uint8_t * buff, uint16_t Length);
void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
