#include "system.h"
#include <stdarg.h>
#include <stdio.h>
#include "usart.h"
#include "main.h"

#define UART1_PRINTF_BUFFER_SIZE    (64)

// I dont like redefine default printf to UART, only RTT
void printf_uart1(const char *format, ...) {
    char buffer[UART1_PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
    }
}

/******************************** Tasks ************************************* */

void System::TasksFunctions::UART1_Printer1( void* arg ){
    while(1){
        printf_uart1("Task 1 - runing");
        xTaskCreate(System::TasksFunctions::UART1_Printer2,
                "UART1_Print2",
                configMINIMAL_STACK_SIZE,
                NULL,
                2,
                &System::TasksHandlers::UART1_Printer2
        );
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void System::TasksFunctions::UART1_Printer2( void* arg ){
    printf_uart1("Task 2 - runing");
    printf_uart1("Task 2 - deleting");
    vTaskDelete(NULL);
}

/******************************** Init ************************************** */

void SystemRun( void ){
    xTaskCreate(System::TasksFunctions::UART1_Printer1,
                "UART1_Print1", 
                configMINIMAL_STACK_SIZE,
                NULL,
                1, 
                &System::TasksHandlers::UART1_Printer1
    );
    vTaskStartScheduler();
}
