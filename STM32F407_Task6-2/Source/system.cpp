#include "system.h"
#include <stdarg.h>
#include <stdio.h>
#include "main.h"

/******************************** Config ************************************ */

#define BTN_JITTER_MS                         (20)
#define BTN_PULL_RATE_HZ                      (50)

/******************************** Internal calcs **************************** */

#define BTN_PULL_RATE_PERIOD_MS (1000/BTN_PULL_RATE_HZ)

/******************************** Common functions ****************************/

bool IsButtonPressed( void ){
    return HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET;
}

void SetLedsState( bool state ){
    GPIO_PinState gpio_state = state ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, gpio_state);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, gpio_state);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, gpio_state);
}

/******************************** Tasks **************************************/

void System::TasksFunctions::BTN_Checker( void* args ){
    bool old_btn_state = false;
    while(1){
        eTaskState blink_task_state;
        uint32_t current_mode;
        System::blink_mode_t next_mode;

        bool btn_state = IsButtonPressed();
        // If button pressed event
        if((old_btn_state != false) || (btn_state != true)){
            goto no_action; // If no press event
        }
        vTaskDelay(pdMS_TO_TICKS(BTN_JITTER_MS));
        if(IsButtonPressed() == false){
            goto no_action; // If button jitter no action
        }
        
        current_mode = static_cast<uint32_t>(System::mode);
        next_mode = static_cast<System::blink_mode_t>(++current_mode);
        System::mode = next_mode == System::MODE_END__ ? System::MODE_OFF : next_mode;
        
        xTimerReset(System::RTOS_Timers::LED_Blink, portMAX_DELAY);
        xTimerStop(System::RTOS_Timers::LED_Blink, portMAX_DELAY);
        switch (System::mode) {
            case System::MODE_OFF:
                SetLedsState(false);
                break;
            case System::MODE_CONSTANT:
                SetLedsState(true);
                break;
            case System::MODE_BLINK_PERIOD_500MS:
                xTimerChangePeriod(System::RTOS_Timers::LED_Blink, pdMS_TO_TICKS(250), portMAX_DELAY);
                xTimerStart(System::RTOS_Timers::LED_Blink, portMAX_DELAY);
                break;
            case System::MODE_BLINK_PERIOD_1S:
                xTimerChangePeriod(System::RTOS_Timers::LED_Blink, pdMS_TO_TICKS(500), portMAX_DELAY);
                xTimerStart(System::RTOS_Timers::LED_Blink, portMAX_DELAY);
                break;
            default: break;
        }
no_action:
        old_btn_state = btn_state;
        vTaskDelay(pdMS_TO_TICKS(BTN_PULL_RATE_PERIOD_MS));
    }
}

/******************************** Callbacks **********************************/

void System::RTOS_TimersCallbacks::LED_Blink( TimerHandle_t xTimer ){
    static bool led_state = false;
    SetLedsState(led_state);
    led_state = !led_state;
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
    System::RTOS_Timers::LED_Blink = xTimerCreate("Led blink",
                portMAX_DELAY,
                pdTRUE, 
                0,
                System::RTOS_TimersCallbacks::LED_Blink
    );
    SetLedsState(false);
    vTaskStartScheduler();
}
