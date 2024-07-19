#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include <stdio.h>
#include <stdlib.h>

#define CCM_RAM __attribute__((section(".ccmram")))

#define ReceivedMsgsLimit   1000

int queueSize=10;

int HighTask_SentMsgs;
int LowTask_1_SentMsgs;
int LowTask_2_SentMsgs;
int Total_SentMsgs;

int HighTask_BlockedMsgs;
int LowTask_1_BlockedMsgs;
int LowTask_2_BlockedMsgs;
int Total_BlockedMsgs;

int ReceivedMsgs;

int HighTask_totalrandPeriod;
int LowTask_1_totalrandPeriod;
int LowTask_2_totalrandPeriod;

int HighTask_callsCount;
int LowTask_1_callsCount;
int LowTask_2_callsCount;

QueueHandle_t queue;
TimerHandle_t timer_HighTask;
TimerHandle_t timer_LowTask_1;
TimerHandle_t timer_LowTask_2;
TimerHandle_t timer_Received;
SemaphoreHandle_t semphr_HighTask;
SemaphoreHandle_t semphr_LowTask_1;
SemaphoreHandle_t semphr_LowTask_2;
SemaphoreHandle_t semphr_Received;


int lowerBounds[6]= {50, 80, 110, 140, 170, 200};
int upperBounds[6]= {150, 200, 250, 300, 350, 400};
int currentBoundIndex=0;

void SenderTaskHigh( void *parameters );
void SenderTaskLow1( void *parameters );
void SenderTaskLow2( void *parameters );
void ReceiverTask( void *parameters );

void SenderTaskHigh_timer_callback(TimerHandle_t xTimer);
void SenderTaskLow1_timer_callback(TimerHandle_t xTimer);
void SenderTaskLow2_timer_callback(TimerHandle_t xTimer);
void ReceiverTask_timer_callback(TimerHandle_t xTimer);

void reset();



void SenderTaskHigh( void *parameters )
{
	char message[20];
	while(1)
	{
		xSemaphoreTake(semphr_HighTask,portMAX_DELAY);

		HighTask_callsCount++;
		int randValue=lowerBounds[currentBoundIndex] + (rand() % ((upperBounds[currentBoundIndex]+1) - lowerBounds[currentBoundIndex]));
		HighTask_totalrandPeriod += randValue;

		sprintf(message,"Time is %i \n",(int)xTaskGetTickCount());

		if ( xQueueSend( queue, message, 0 ) == pdPASS )
		{
			HighTask_SentMsgs++;
			xTimerChangePeriod(timer_HighTask, pdMS_TO_TICKS(randValue), 0);
		}
		else
		{
			HighTask_BlockedMsgs++;
			xTimerChangePeriod(timer_HighTask, pdMS_TO_TICKS(randValue), 0);
		}
	}
}

void SenderTaskLow1( void *parameters )
{
	char message[20];
	while(1)
	{
		xSemaphoreTake(semphr_LowTask_1,portMAX_DELAY);

		LowTask_1_callsCount++;
		int randValue=lowerBounds[currentBoundIndex] + (rand() % ((upperBounds[currentBoundIndex]+1) - lowerBounds[currentBoundIndex]));
		LowTask_1_totalrandPeriod += randValue;
		sprintf(message,"Time is %i",(int)xTaskGetTickCount());

		if ( xQueueSend( queue, message, 0 ) == pdPASS )
		{
			LowTask_1_SentMsgs++;
			xTimerChangePeriod(timer_LowTask_1, pdMS_TO_TICKS(randValue), 0);
		}
		else
		{
			LowTask_1_BlockedMsgs++;
			xTimerChangePeriod(timer_LowTask_1, pdMS_TO_TICKS(randValue), 0);
		}

	}
}


void SenderTaskLow2( void *parameters )
{
	char message[20];
	while(1)
	{
		xSemaphoreTake(semphr_LowTask_2,portMAX_DELAY);

		LowTask_2_callsCount++;
		int randValue=lowerBounds[currentBoundIndex] + (rand() % ((upperBounds[currentBoundIndex]+1) - lowerBounds[currentBoundIndex]));
		LowTask_2_totalrandPeriod += randValue;

		sprintf(message,"Time is %i",(int)xTaskGetTickCount());

		if ( xQueueSend( queue, message, 0 ) == pdPASS )
		{
			LowTask_2_SentMsgs++;
			xTimerChangePeriod(timer_LowTask_2, pdMS_TO_TICKS(randValue), 0);
		}
		else
		{
			LowTask_2_BlockedMsgs++;
			xTimerChangePeriod(timer_LowTask_2, pdMS_TO_TICKS(randValue), 0);
		}
	}
}



void ReceiverTask( void *parameters )
{
	char message[20];
	while(1)
	{
		xSemaphoreTake(semphr_Received,portMAX_DELAY);
		if( xQueueReceive( queue, message, 0 ) == pdPASS )
		{
			ReceivedMsgs++;
			printf("%s\n",message);
		}
	}
}

void SenderTaskHigh_timer_callback(TimerHandle_t xTimer) {
	xSemaphoreGive(semphr_HighTask);
}

void SenderTaskLow1_timer_callback(TimerHandle_t xTimer) {
	xSemaphoreGive(semphr_LowTask_1);
}

void SenderTaskLow2_timer_callback(TimerHandle_t xTimer) {
	xSemaphoreGive(semphr_LowTask_2);
}

void ReceiverTask_timer_callback(TimerHandle_t xTimer) {
	if(ReceivedMsgs==ReceivedMsgsLimit)
	{
		reset();
	}
	else
	{
		xSemaphoreGive(semphr_Received);
	}
}

void reset()
{
	int total_sent = HighTask_SentMsgs + LowTask_1_SentMsgs + LowTask_2_SentMsgs;
	int total_blocked = HighTask_BlockedMsgs + LowTask_1_BlockedMsgs + LowTask_2_BlockedMsgs;

	int HighTask_avgPeriod = HighTask_totalrandPeriod / HighTask_callsCount;
	int LowTask_1_avgPeriod = LowTask_1_totalrandPeriod / LowTask_1_callsCount;
	int LowTask_2_avgPeriod = LowTask_2_totalrandPeriod / LowTask_2_callsCount;
	int TotalTasks_avgPeriod = (HighTask_totalrandPeriod + LowTask_1_totalrandPeriod + LowTask_2_totalrandPeriod ) / (HighTask_callsCount + LowTask_1_callsCount + LowTask_2_callsCount);

	printf("Iteration %i ,Queue %i \n",currentBoundIndex + 1, queueSize);
	printf("Total sent messages = %i \n",total_sent);
	printf("Total blocked messages = %i\n",total_blocked);
	printf ("High priority task sent = %i \n", HighTask_SentMsgs);
	printf ("Low priority task1 sent = %i \n", LowTask_1_SentMsgs);
	printf ("Low priority task2 sent = %i \n", LowTask_2_SentMsgs);
	printf ("High priority task blocked = %i \n", HighTask_BlockedMsgs);
	printf ("Low priority task1 blocked = %i \n", LowTask_1_BlockedMsgs);
	printf ("Low priority task2 blocked = %i \n", LowTask_2_BlockedMsgs);


	HighTask_SentMsgs=0;
	LowTask_1_SentMsgs=0;
	LowTask_2_SentMsgs=0;

	HighTask_BlockedMsgs=0;
	LowTask_1_BlockedMsgs=0;
	LowTask_2_BlockedMsgs=0;

	ReceivedMsgs=0;

	HighTask_totalrandPeriod=0;
	LowTask_1_totalrandPeriod=0;
	LowTask_2_totalrandPeriod=0;

	HighTask_callsCount=0;
	LowTask_1_callsCount=0;
	LowTask_2_callsCount=0;

	xQueueReset(queue);

	currentBoundIndex++;



	if(currentBoundIndex <= 5)
	{
        int randValueHigh = lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));
        int randValueLow1 = lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));
        int randValueLow2 = lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));

        HighTask_totalrandPeriod += randValueHigh;
        LowTask_1_totalrandPeriod += randValueLow1;
        LowTask_2_totalrandPeriod += randValueLow2;

        xTimerChangePeriod(timer_HighTask, pdMS_TO_TICKS(randValueHigh), 0);
        xTimerChangePeriod(timer_LowTask_1, pdMS_TO_TICKS(randValueLow1), 0);
        xTimerChangePeriod(timer_LowTask_2, pdMS_TO_TICKS(randValueLow2), 0);


	}
	else
	{
		printf("Game Over\n");
		xTimerDelete(timer_HighTask,0);
		xTimerDelete(timer_LowTask_1,0);
		xTimerDelete(timer_LowTask_2,0);
		xTimerDelete(timer_Received,0);
		exit(0);
		vTaskEndScheduler();

	}
}

void reset_initialSetup()
{
	HighTask_SentMsgs=0;
	LowTask_1_SentMsgs=0;
	LowTask_2_SentMsgs=0;
	Total_SentMsgs=0;

	HighTask_BlockedMsgs=0;
	LowTask_1_BlockedMsgs=0;
	LowTask_2_BlockedMsgs=0;
	Total_BlockedMsgs=0;

	ReceivedMsgs=0;

	xQueueReset( queue );

	HighTask_totalrandPeriod=0;
	LowTask_1_totalrandPeriod=0;
	LowTask_2_totalrandPeriod=0;

	HighTask_callsCount=0;
	LowTask_1_callsCount=0;
	LowTask_2_callsCount=0;
}

int main()
{
	queue=xQueueCreate(queueSize, sizeof(char)*20);

	reset_initialSetup();

	xTaskCreate(SenderTaskHigh, "High", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate(SenderTaskLow1, "Low1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(SenderTaskLow2, "Low2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(ReceiverTask, "ReceivedMsgs", configMINIMAL_STACK_SIZE, NULL, 3, NULL);


	semphr_HighTask=xSemaphoreCreateBinary();
	semphr_LowTask_1=xSemaphoreCreateBinary();
	semphr_LowTask_2=xSemaphoreCreateBinary();
	semphr_Received=xSemaphoreCreateBinary();

	int randValueHigh=lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));
	int randValueLow1=lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));
	int randValueLow2=lowerBounds[currentBoundIndex] + (rand() % (upperBounds[currentBoundIndex] - lowerBounds[currentBoundIndex] + 1));

	HighTask_totalrandPeriod += randValueHigh;
	LowTask_1_totalrandPeriod += randValueLow1;
	LowTask_2_totalrandPeriod += randValueLow2;

	HighTask_callsCount++;
	LowTask_1_callsCount++;
	LowTask_2_callsCount++;

	timer_HighTask=xTimerCreate("SenderTimerHigh", pdMS_TO_TICKS(randValueHigh), pdTRUE, (void*)0, SenderTaskHigh_timer_callback);
	timer_LowTask_1=xTimerCreate("SenderTimerLow1", pdMS_TO_TICKS(randValueLow1), pdTRUE, (void*)0, SenderTaskLow1_timer_callback);
	timer_LowTask_2=xTimerCreate("SenderTimerLow2", pdMS_TO_TICKS(randValueLow2), pdTRUE, (void*)0, SenderTaskLow2_timer_callback);
	timer_Received=xTimerCreate("ReceiverTimer", pdMS_TO_TICKS(100), pdTRUE, (void*)0, ReceiverTask_timer_callback);

	xTimerStart(timer_HighTask, 0);
	xTimerStart(timer_LowTask_1, 0);
	xTimerStart(timer_LowTask_2, 0);
	xTimerStart(timer_Received, 0);

	vTaskStartScheduler();

}
#pragma GCC diagnostic pop



/********************************************************************************************************************** */

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
void vApplicationIdleHook( void )
{
	volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
void vApplicationTickHook(void) {
}
StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
