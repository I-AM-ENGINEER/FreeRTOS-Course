#include "system.h"
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "usart.h"

/*
Размер кучи - 2048 байт
Атомарный элемент элемента стека - uint32_t (4 байта)
Выравнивание элементов стека - 8 байт

Аллокация Task1 и Idle
Занято 1224 байт из 2048 (свободно 824):
[0...7] 	8 байт - инфа о блоке и его размер
[8...527]	520 байт - стек задачи 1 (130*4)
[528...535]	8 байт - инфа о блоке и его размер
[536...607]	72 байт - служебная информация задачи (приоритет, события и т.д.)
[608...615]	8 байт - инфа о блоке и его размер
[616...1135]	520 байт - стек задачи ожидания (130*4)
[1136...1143]	8 байт - инфа о блоке и его размер
[1144...1215]	72 байт - служебная информация задачи ожидания (приоритет, события и т.д.)
[1216...1223]	8 байт - инфа о блоке и его размер
Аллокация Task2
Занято 1832 байт из 2048 (свободно 216):
[1224...1743]	520 байт - стек задачи 2 (130*4)
[1744...1751]	8 байт - инфа о блоке и его размер
[1752...1823]	72 байт - служебная информация задачи (приоритет, события и т.д.)
[1824...1831]	8 байт - инфа о блоке и его размер
Аллокация буфера
Занято 1880 байт из 2048 (свободно 168):
[1832...1871]	40 байт - Буфер 20 элементов uint16_t
[1872...1879]	8 байт - инфа о блоке и его размер
*/

#define UART1_PRINTF_BUFFER_SIZE    (64)

/******************************** Common functions ****************************/

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

/******************************** Tasks **************************************/

void System::TasksFunctions::Task1( void* arg ){
    TickType_t timestamp = xTaskGetTickCount();
    while(1){
        size_t free_heap = xPortGetFreeHeapSize();
        printf_uart1("Heap size before task 2 allocation: %lu\n", free_heap);
        xTaskCreate(System::TasksFunctions::Task2,
                    "Task2",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    1,
                    &System::TasksHandlers::Task2
        );
        //free_heap = xPortGetFreeHeapSize();
        //printf_uart1("Heap size after task 2 allocation: %lu\n", free_heap);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void System::TasksFunctions::Task2( void* arg ){
    size_t free_heap = xPortGetFreeHeapSize();
    printf_uart1("Heap size before buffer allocation: %lu\n", free_heap);
    uint16_t* buff = static_cast<uint16_t*>(pvPortMalloc(sizeof(uint16_t)*20));
    free_heap = xPortGetFreeHeapSize();
    printf_uart1("Heap size after buffer allocation: %lu\n", free_heap);
    vPortFree(buff);
    free_heap = xPortGetFreeHeapSize();
    printf_uart1("Heap size after buffer free: %lu\n", free_heap);
    vTaskDelete(NULL);
}

/******************************** Init ***************************************/

void SystemRun( void ){
  xTaskCreate(System::TasksFunctions::Task1,
              "Task1",
              configMINIMAL_STACK_SIZE,
              NULL,
              1,
              &System::TasksHandlers::Task1
  );  
  vTaskStartScheduler();
}
