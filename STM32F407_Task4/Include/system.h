#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus

namespace System{
    namespace TasksFunctions{
        void Task1( void* args );
        void Task2( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t Task1;
        TaskHandle_t Task2;
    }
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
