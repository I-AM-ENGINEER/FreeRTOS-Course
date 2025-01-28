#include "system.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

TaskHandle_t xBlinkTaskHandler;

void vBlinkLD3Task( void* arg ){
  while(1){
    vTaskDelay(pdMS_TO_TICKS(100));
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
  }
}

void vBlinkLD4Task( void* arg ){
  while(1){
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin);
  }
}

void SystemRun( void ){
  xTaskCreate(vBlinkLD3Task, "BlinkLD3", configMINIMAL_STACK_SIZE, NULL, 0, &xBlinkTaskHandler);
  xTaskCreate(vBlinkLD4Task, "BlinkLD4", configMINIMAL_STACK_SIZE, NULL, 0, &xBlinkTaskHandler);
  vTaskStartScheduler();
}
