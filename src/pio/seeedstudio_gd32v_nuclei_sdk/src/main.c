/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */
#include "task.h"     /* RTOS task related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "nuclei_sdk_soc.h"

// S/Wタイマの周期
#define mainSOFTWARE_TIMER_PERIOD_MS    pdMS_TO_TICKS(1000)
// FreeRTOSのキューの最大長
#define mainQUEUE_LENGTH                (1)

extern void idle_task(void);
static void xSoftware1000msTimerCallback(TimerHandle_t xTimer);

static QueueHandle_t xQueue = NULL;
static TaskHandle_t MainTask_Handler;
static TaskHandle_t LedTask_Handler;
// static uint32_t s_cnt = 0;

void main_task(void *pvParameters);
void led_task(void *pvParameters);
static void inithardware(void);
static void initfreertos(void);

static void inithardware(void)
{
    // GPIO
    // TODO:ボタンとLEDのGPIO初期化

    // S/WTimer
    TimerHandle_t xSoftware1000msTimer = NULL;
    xSoftware1000msTimer = xTimerCreate((const char *)"1000msTimer",
                                        mainSOFTWARE_TIMER_PERIOD_MS,
                                        pdTRUE, (void *)0,
                                        xSoftware1000msTimerCallback);
    xTimerStart(xSoftware1000msTimer, 0);
}

void main_task(void *pvParameters)
{
    printf("Main Task Inited!\r\n");

    while(1)
    {
        // TODO:メインタスク処理
        vTaskDelay(100);
    }
}

void led_task(void *pvParameters)
{
    printf("LED Task Inited!\r\n");

    while(1)
    {
        // TODO:LEDタスク処理
        vTaskDelay(300);
    }
}

static void initfreertos(void)
{
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    if (xQueue == NULL) {
        printf("Unable to create xQueue due to low memory.\n");
        while(1);
    }

    xTaskCreate((TaskFunction_t)main_task,
                (const char *)"main_task",
                (uint16_t)512,
                (void *)NULL,
                (UBaseType_t)2,
                (TaskHandle_t *)&MainTask_Handler);

    xTaskCreate((TaskFunction_t)led_task,
                (const char *)"led_task",
                (uint16_t)256,
                (void *)NULL,
                (UBaseType_t)1,
                (TaskHandle_t *)&LedTask_Handler);
}

int main(void)
{
    inithardware();
    initfreertos();
    vTaskStartScheduler();

    return 0;
}

// FreeRTOS S/Wタイマ(1000ms)
static void xSoftware1000msTimerCallback(TimerHandle_t xTimer)
{
    asm volatile ("nop");
}

void vApplicationTickHook(void)
{
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
    1 in FreeRTOSConfig.h.

    "Give" the semaphore on every 500th tick interrupt. */

    /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).  The syntax required to context switch from
    an interrupt is port dependent, so check the documentation of the port you
    are using.

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    printf("malloc failed\n");
    while(1);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    printf("Stack Overflow\n");
    while(1);
}
/*-----------------------------------------------------------*/

extern UBaseType_t uxCriticalNesting;
void vApplicationIdleHook(void)
{
    // volatile size_t xFreeStackSpace;
    /* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
    FreeRTOSConfig.h.

    This function is called on each cycle of the idle task.  In this case it
    does nothing useful, other than report the amount of FreeRTOS heap that
    remains unallocated. */
    /* By now, the kernel has allocated everything it is going to, so
    if there is a lot of heap remaining unallocated then
    the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
    reduced accordingly. */
}
