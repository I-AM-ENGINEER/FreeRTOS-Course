Задание
Реализовать многозадачную систему, в которой сообщения между задачами передаются через общую
очередь. Основные условия:
1. Система создаётся с тремя задачами и одной очередью.
2. Очередь имеет размер 100. Элементом очереди является структура, пример:
typedef enum {
MSG_ID_KEYBOARD = 0,
MSG_ID_ADC
} msgID;
typedef struct {
msgID id;
uint16_t msgVal;
} queueMsg;
3. Задача 1 выполняет опрос пользовательской кнопки на отладочной плате. При нажатии формируется
сообщение в очередь с идентификатором события нажатия кнопки MSG_ID_KEYBOARD.
4. Задача 2 выполняет запуск АЦП и преобразование по выбранному каналу с периодом 500мс. Результат
преобразования упаковывает в сообщение для очереди с идентификатором события АЦП
MSG_ID_ADC и численным значением преобразования.
5. Задача 3 выполняет чтение из очереди (таймаут чтения 1000 мс). При получении сообщения, для
каждого идентификатора выполняется соответствующее действие:
 для события MSG_ID_KEYBOARD – переключается состояние пользовательского
светодиода на противоположное (исходное состояние для светодиода после загрузки
системы – выключен);
 для события MSG_ID_ADC – обновляется значение локальной переменной, содержащей
значение предыдущего преобразования и происходит вывод этого значения через терминал
на ПК в виде форматной строки «ADC = %d\n».
6. Выполнить сборку, запуск и тестирование проекта на отладочной плате.
Критерии готовности (Definition of Done)
1. Проект собирается и загружается в отладочную плату без ошибок и предупреждений.
2. Начальные условия при включении системы соблюдаются.
3. По последовательному порту передаётся значение текущего преобразования АЦП. При
подключении рабочего канала АЦП на вывод GND и вывод +3V3 наблюдаются соответствующие
изменения в выводе данных (близкое к нулю для GND, близкое к 4096 для +3V3, допускается
вывод в виде значения напряжения).
4. Нажатия на пользовательскую кнопку обрабатываются корректно и однозначно определяют
состояние светодиода, многократные срабатывания не происходят.
5. При однократном нажатии на кнопку светодиод переключает своё состояние, при этом не
нарушается вывод сообщений о преобразовании АЦП.

Вывод отладки - RS232 (CN1)
Светодиод - LED1
Кнопка - S1