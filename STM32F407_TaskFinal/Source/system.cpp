#include "system.h"
#include "main.h"
#include "rng.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>


uint8_t Device::uart_rx_byte_buffer = 0;
UART_STM32 Device::uart1(&huart1, 128);
uint8_t Game::_difficulty = 0;
uint8_t Game::_patterns[GAME_MAX_LEVEL] = {0};
QueueHandle_t Device::keyboard_events = nullptr;
TimerHandle_t Device::keyboard_checker_timer = nullptr;

static StaticTask_t xTaskBuffer;
static StackType_t xStack[configMINIMAL_STACK_SIZE];


/******************** Boilerplate for allow static allocation ************************/

static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskBuffer;
static StackType_t xTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskBuffer;
    *ppxTimerTaskStackBuffer = xTimerTaskStack;
    *pulTimerTaskStackSize = sizeof(xTimerTaskStack) / sizeof(StackType_t);
}


/******************************** Game class **************************************/

// Game main cycle
void Game::MainTask( void* args ){
    while(1){
        Device::HW::SetLeds(0x00);
        Device::uart1.Send((const uint8_t*)GAME_MENU_START_TEXT, strlen(GAME_MENU_START_TEXT), 1000);
        while (true) {
            char byte = 0;
            if (Device::uart1.Recv(&byte, 1, 0) > 0) {
                switch (byte) {
                    case '1': SetDifficulty(0); break; 
                    case '2': SetDifficulty(1); break;
                    default: vTaskDelay(1); continue;
                }
                break;
            }
            vTaskDelay(1);
        }
        Device::uart1.Send((const uint8_t*)GAME_MENU_PRERUN_TEXT, strlen(GAME_MENU_PRERUN_TEXT), 1000);
        Game::Start();
    }
}


void Game::SetDifficulty( uint8_t difficulty ){
    _difficulty = difficulty;
}

uint32_t Game::GetLightOnTimeMs( void ){
    uint32_t lighton;
    switch(_difficulty) {
        case 0: lighton = GAME_LIGHT_TIME_DIFFICULTY1_MS; break;
        case 1: lighton = GAME_LIGHT_TIME_DIFFICULTY2_MS; break;
        default: lighton = 0; break;
    }
    return lighton;
}

void Game::DisplayPatterns( uint16_t current_level ){
    for(uint16_t i = 0; i < current_level; i++){
        Device::HW::SetLeds(0x00);
        vTaskDelay(pdMS_TO_TICKS(GAME_DELAY_BETWEEN_FLASHES_MS));
        Device::HW::SetLeds(1 << _patterns[i]);
        vTaskDelay(pdMS_TO_TICKS(GetLightOnTimeMs()));
    }
    Device::HW::SetLeds(0x00);
}

bool Game::CheckInput( uint16_t current_level ){
    xQueueReset(Device::keyboard_events); // Clear previous input
    for(uint16_t i = 0; i < current_level; i++){
        keyboard_message_t msg;
        // No one reason for use program timers for user input as described in 10 task condition
        BaseType_t res = xQueueReceive(Device::keyboard_events, &msg, GAME_MAX_ENTER_TIME_MS);
        if(res != pdTRUE){
            Device::uart1.Send((const uint8_t*)GAME_GAMEOVER_TIMEOUT_TEXT, strlen(GAME_GAMEOVER_TIMEOUT_TEXT), 1000);
            return false;
        }
        if(msg.event_type == KEYBOARD_EVENT_PRESS){
            if(msg.button_num == _patterns[i]){
                Device::HW::SetLeds(1 << _patterns[i]);
                vTaskDelay(pdMS_TO_TICKS(GAME_CORRECT_INPUT_LEDON_TIME_MS));
                Device::HW::SetLeds(0x00);
            }else{
                Device::uart1.Send((const uint8_t*)GAME_GAMEOVER_INCORRECT_TEXT, strlen(GAME_GAMEOVER_INCORRECT_TEXT), 1000);
                return false;
            }
        }
    }
    return true;
}

void Game::Start( void ){
    // Generate random patterns
    for(size_t i = 0; i < sizeof(_patterns)/sizeof(*_patterns); i++){
        _patterns[i] = rand() % GAME_BUTTONS_COUNT;
    }

    vTaskDelay(pdMS_TO_TICKS(GAME_PRELAUNCH_DELAY_MS));
    for(uint16_t i = 0; i < GAME_MAX_LEVEL; i++){
        vTaskDelay(pdMS_TO_TICKS(GAME_DELAY_BETWEEN_ROUNDS));
        uint16_t current_level = i+1;
        DisplayPatterns(current_level);
        bool input_correct = CheckInput(current_level);
        if(!input_correct){
            return;
        }
    }
    Device::uart1.Send((const uint8_t*)GAME_WIN_TEXT, strlen(GAME_WIN_TEXT), 1000);
    Device::HW::SetLeds((1 << GAME_BUTTONS_COUNT) - 1);
    vTaskDelay(pdMS_TO_TICKS(GAME_WIN_FLASH_TIME_MS));
    Device::HW::SetLeds(0x00);
}

/******************************** Hardware class ***************************************/

void Device::HW::SetLeds( uint8_t mask ){
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, mask & (1 << 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, mask & (1 << 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, mask & (1 << 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

uint8_t Device::HW::ReadButtons( void ){
    uint8_t mask = 0;
    mask |= HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET ? (1 << 0) : 0;
    mask |= HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET ? (1 << 1) : 0;
    mask |= HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET ? (1 << 2) : 0;
    return mask;
}

static void keyboard_checker_timer_callback( TimerHandle_t xTimer ) {
    static uint8_t stable_state = 0;
    uint8_t raw = Device::HW::ReadButtons();

    for (int i = 0; i < GAME_BUTTONS_COUNT; i++) {
        uint8_t current_bit = (raw >> i) & 0x01;
        uint8_t previous_bit = (stable_state >> i) & 0x01;

        if (!previous_bit && current_bit) {
            keyboard_message_t msg;
            msg.event_type = KEYBOARD_EVENT_PRESS;
            msg.button_num = i;
            xQueueSend(Device::keyboard_events, &msg, 0);
        }
        
        if (current_bit) {
            stable_state |= (1 << i);
        } else {
            stable_state &= ~(1 << i);
        }
    }
}

void SystemRun( void ){
    huart1.Init.BaudRate = 115200;
    HAL_UART_Init(&huart1);
    HAL_UART_Receive_IT(&huart1, &Device::uart_rx_byte_buffer, 1);
    MX_RNG_Init();
    srand(HAL_RNG_GetRandomNumber(&hrng));
    xTaskCreateStatic(Game::MainTask, "main", configMINIMAL_STACK_SIZE, NULL, 1, xStack, &xTaskBuffer);
    Device::keyboard_events = xQueueCreate(KEYBOARD_QUEUE_LENGTH, sizeof(keyboard_message_t));
    Device::keyboard_checker_timer = xTimerCreate("BTN checker", pdMS_TO_TICKS(KEYBOARD_POOL_PERIOD_MS), pdTRUE, (void*)0, keyboard_checker_timer_callback);
    xTimerStart(Device::keyboard_checker_timer, 0);
    vTaskStartScheduler();
}

/******************************** ISR Callbacks ******************************/

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart ){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_UART_Receive_IT(&huart1, &Device::uart_rx_byte_buffer, 1);
    Device::uart1.ISR_ReceiveByte(Device::uart_rx_byte_buffer);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
