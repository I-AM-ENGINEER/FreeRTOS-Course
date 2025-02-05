#include "system.h"
#include <stdarg.h>
#include <stdio.h>
#include "main.h"

/******************************** Config ************************************ */

#define BTN_JITTER_MS                         (20)
#define BTN_PULL_RATE_HZ                      (50)
#define LED_OFF_TIMEOUT_MS                    (4000)

/******************************** Internal calcs **************************** */

#define BTN_PULL_RATE_PERIOD_MS (1000/BTN_PULL_RATE_HZ)

/******************************** Common functions ****************************/

bool IsButtonPressed( void ){
    return HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET;
}

/******************************** Tasks **************************************/

void System::TasksFunctions::BTN_Checker( void* args ){
    bool old_btn_state = false;
    while(1){
        eTaskState blink_task_state;
        bool btn_state = IsButtonPressed();
        // If button pressed event
        if((old_btn_state != false) || (btn_state != true)){
            goto no_action; // If no press event
        }
        vTaskDelay(pdMS_TO_TICKS(BTN_JITTER_MS));
        if(IsButtonPressed() == false){
            goto no_action; // If button jitter no action
        }
        
        
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        xTimerReset(System::RTOS_Timers::LED_Off, 0);
no_action:
        old_btn_state = btn_state;
        vTaskDelay(pdMS_TO_TICKS(BTN_PULL_RATE_PERIOD_MS));
    }
}

/******************************** Callbacks **********************************/

void System::RTOS_TimersCallbacks::LED_Off( TimerHandle_t xTimer ){
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

/******************************** Init ***************************************/

void SystemRun( void ){
    xTaskCreate(System::TasksFunctions::BTN_Checker, 
                "BTN checker",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::BTN_Checker
    );
    System::RTOS_Timers::LED_Off = xTimerCreate("Led off",
                pdTICKS_TO_MS(LED_OFF_TIMEOUT_MS),
                pdFALSE, 
                0, 
                System::RTOS_TimersCallbacks::LED_Off
    );
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    vTaskStartScheduler();
}
