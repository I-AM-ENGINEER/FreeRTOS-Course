#include "system.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

void vBlinkLD3Task( void* arg ){
  while(1){
    vTaskDelay(pdMS_TO_TICKS(100));
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  }
}

void vBlinkLD4Task( void* arg ){
  while(1){
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  }
}

void SystemRun( void ){
  xTaskCreate(vBlinkLD3Task, "Blink LED1", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
  xTaskCreate(vBlinkLD4Task, "Blink LED2", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
  vTaskStartScheduler();
}
