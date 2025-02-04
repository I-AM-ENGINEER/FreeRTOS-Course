#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
namespace System{
    namespace TasksFunctions{
        void UART1_Printer1( void* arg );
        void UART1_Printer2( void* arg );
    }
    namespace TasksHandlers{
        TaskHandle_t UART1_Printer1;
        TaskHandle_t UART1_Printer2;
    }
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
