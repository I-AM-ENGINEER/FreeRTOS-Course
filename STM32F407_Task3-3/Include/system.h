#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
namespace System{
    namespace TasksFunctions{
        void LedBlinker( void* arg );
        void ButtonChecker( void* arg );
    }
    namespace TasksHandlers{
        TaskHandle_t LedBlinker;
        TaskHandle_t ButtonChecker;
    }
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
