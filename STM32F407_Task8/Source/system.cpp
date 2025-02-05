#include "system.h"
#include "main.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

/******************************** Config ************************************ */

#define TASK1_PERIOD_MS       (1000)
#define TASK2_PERIOD_MIN_MS   (100)
#define TASK2_PERIOD_MAX_MS   (500)

/******************************** Tasks **************************************/

void System::TasksFunctions::UART_Sender1( void* args ){
    char buff[50];
    TickType_t timestamp = xTaskGetTickCount();
    while(1) {
        vTaskDelayUntil(&timestamp, pdMS_TO_TICKS(TASK1_PERIOD_MS));
        snprintf(buff, sizeof(buff), "Task 1 working, time: %lu\n", timestamp);
        System::uart1.Send((uint8_t*)buff, strlen(buff), TASK1_PERIOD_MS);
    }
}

void System::TasksFunctions::UART_Sender2( void* args ){
    char buff[50];
    TickType_t timestamp = xTaskGetTickCount();
    while(1) {
        uint32_t t = TASK2_PERIOD_MIN_MS + (rand() % (TASK2_PERIOD_MAX_MS - TASK2_PERIOD_MIN_MS));
        vTaskDelayUntil(&timestamp, pdMS_TO_TICKS(t));
        snprintf(buff, sizeof(buff), "Task 2 working, time: %lu\n", timestamp);
        System::uart1.Send((uint8_t*)buff, strlen(buff), t);
    }
}

/******************************** Init ***************************************/

void SystemRun( void ){
    xTaskCreate(System::TasksFunctions::UART_Sender1, 
                "BTN checker",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::UART_Sender1
    );

    xTaskCreate(System::TasksFunctions::UART_Sender2, 
                "BTN checker",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::UART_Sender2
    );
    srand(0);
    vTaskStartScheduler();
}
