#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef enum {
    MSG_ID_KEYBOARD = 0,
    MSG_ID_ADC,
} msg_id_t;

typedef struct {
    msg_id_t id;
    uint16_t val;
} queue_msg_t;

#ifdef __cplusplus

namespace System{
    namespace TasksFunctions{
        void ADC_Reader( void* args );
        void BTN_Checker( void* args );
        void QueueHandler( void* args );
    }
    namespace TasksHandlers{
        TaskHandle_t ADC_Reader;
        TaskHandle_t BTN_Checker;
        TaskHandle_t QueueHandler;
    }
    namespace Queues{
        QueueHandle_t msgs;
    }
}

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
