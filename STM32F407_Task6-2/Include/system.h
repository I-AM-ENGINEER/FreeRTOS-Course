#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#ifdef __cplusplus

namespace System{
    namespace TasksFunctions{
        void BTN_Checker( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t BTN_Checker;
    }
    namespace RTOS_Timers{
        TimerHandle_t LED_Blink;
    }
    namespace RTOS_TimersCallbacks{
        void LED_Blink( TimerHandle_t xTimer );
    }
    typedef enum{
        MODE_OFF = 0,
        MODE_CONSTANT,
        MODE_BLINK_PERIOD_1S,
        MODE_BLINK_PERIOD_500MS,
        MODE_END__,
    } blink_mode_t;

    blink_mode_t mode = MODE_OFF;
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
