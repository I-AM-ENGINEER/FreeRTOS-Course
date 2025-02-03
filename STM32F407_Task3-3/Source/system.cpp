#include "system.h"
#include "main.h"
#include <stdbool.h>

/******************************** Config ************************************ */

#define BTN_JITTER_MS           (20)
#define BTN_PULL_RATE_HZ        (50)
#define LED_BLINK_FREQUENCY_HZ  (10)

/******************************** Internal calcs **************************** */

#define LED_BLINK_PERIOD_MS     (1000/2/LED_BLINK_FREQUENCY_HZ)
#define BTN_PULL_RATE_PERIOD_MS (1000/BTN_PULL_RATE_HZ)

/******************************** Tasks ************************************* */

void System::TasksFunctions::LedBlinker( void* arg ){
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    vTaskSuspend(NULL);
    while(1){
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_PERIOD_MS));
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }
}

bool IsButtonPressed( void ){
    return HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET;
}

void System::TasksFunctions::ButtonChecker( void* arg ){
    static bool old_btn_state = false;
    eTaskState blink_task_state;
    while(1){
        bool btn_state = IsButtonPressed();
        // If button pressed event
        if((old_btn_state == false) && (btn_state == true)){
            vTaskDelay(pdMS_TO_TICKS(BTN_JITTER_MS));
            if(IsButtonPressed() == false){
                goto no_action; // If button jitter no action
            }
            blink_task_state = eTaskGetState(System::TasksHandlers::LedBlinker);
            if(blink_task_state != eSuspended){
                goto no_action; // If task isnt suspended for some reason no action
            }
            vTaskResume(System::TasksHandlers::LedBlinker);
        }// If button unpressed event
        else if((old_btn_state == true) && (btn_state == false)){
            vTaskDelay(pdMS_TO_TICKS(BTN_JITTER_MS));
            if(IsButtonPressed() == true){
                goto no_action; // If button jitter no action
            }
            blink_task_state = eTaskGetState(System::TasksHandlers::LedBlinker);
            if(blink_task_state != eRunning){
                goto no_action; // If task isnt running for some reason no action
            }
            vTaskSuspend(System::TasksHandlers::LedBlinker);
        }
no_action:
        old_btn_state = btn_state;
        vTaskDelay(pdMS_TO_TICKS(BTN_PULL_RATE_PERIOD_MS));
    }
}

/******************************** Init ************************************** */

void SystemRun( void ){
    xTaskCreate(System::TasksFunctions::ButtonChecker,
                "BTN checker", 
                configMINIMAL_STACK_SIZE,
                NULL,
                1, 
                &System::TasksHandlers::ButtonChecker
    );
    // Suspended after run
    xTaskCreate(System::TasksFunctions::LedBlinker,
                "LED blinker", 
                configMINIMAL_STACK_SIZE,
                NULL,
                1, 
                &System::TasksHandlers::LedBlinker
    );
    vTaskStartScheduler();
}
