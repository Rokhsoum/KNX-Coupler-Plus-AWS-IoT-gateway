/*
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uart2echo.c ========
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#include "uart2echo.h"
#include "knx_link.h"
#include "knx_app.h"
#include "knx_link_adapter.h"
#include "knx_link_internal.h"
#include "knx_commissioning_data.h"

#define UART_CH_NAME        10
typedef struct {
    int         channel;
    UART_Handle uart;
    char        name[UART_CH_NAME];
} UART_Task_Arg_t;

//static UART_Task_Arg_t taskUplink_args, taskDownlink_args;
//static const char echoPrompt[] = "Echoing characters:\r\n";
//static const char taskPrompt[] = "Task ready (UART driver, channel %01d, name %s):\r\n";
//#define TASK_PROMPT_MAX     (sizeof(taskPrompt)-4-2+1+UART_CH_NAME+1)

//static void uartEcho(void *arg);

TaskHandle_t create_task(TaskFunction_t func, void *arg, char *taskname, int priority) {
    bool schedulerStarted;
    TaskHandle_t handle;

    schedulerStarted = (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) ? false : true;
    if (schedulerStarted) {
        /* Disable the scheduler */
        vTaskSuspendAll();
    }
    if (xTaskCreate(func, taskname, THREADSTACKSIZE, arg, priority, &handle) != pdPASS) {
        handle = NULL;
    }
    if (schedulerStarted) {
        /* Re-enable the scheduler */
        xTaskResumeAll();
    }
    return handle;
}

#if 0
static void uartEcho(void *arg) {
    char        input;
    char        prompt[TASK_PROMPT_MAX];
    UART_Task_Arg_t *task_arg = (UART_Task_Arg_t *)arg;
    UART_Handle uart = task_arg->uart;
    size_t      bytesRead;
    size_t      bytesWritten = 0;

    snprintf(prompt, sizeof(prompt), taskPrompt, task_arg->channel, task_arg->name);
    bytesWritten = UART_write(uart, prompt, strlen(prompt));

    /* Loop forever echoing */
    while (1) {
        do {
            bytesRead = UART_read(uart, &input, 1);
            if (bytesRead <= 0) {
                vTaskDelay(1);
            }
        } while (bytesRead <= 0);
        do {
            bytesWritten = UART_write(uart, &input, 1);
            if (bytesWritten <= 0) {
                vTaskDelay(1);
            }
        } while (bytesWritten <= 0);
        vTaskDelay(1);
    }
}
#endif
/*
 *  ======== mainThread ========
 */
void mainThread(void *arg0)
{

    //UART_Params uartParams;
    UART_Handle uartUplink;
    UART_Handle uartDownlink;
    Button_Params buttonParams;
    Button_Handle buttonLeft;
    Button_Handle buttonRight;
    LED_Params ledParams;
    //LED_Handle ledGreen;
    //LED_Handle ledYellow;
    LED_Handle ledRed;
    struct knxLinkHandle_s* knxlink_handle1;
    struct knxLinkHandle_s* knxlink_handle2;
    //size_t      bytesWritten;
    commissioning_data_t comm_data;

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_LED_0_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    /* Turn off user LED */
    GPIO_write(CONFIG_LED_0_GPIO, CONFIG_GPIO_LED_OFF);

#if 0
    UART_Params_init(&uartParams);
    uartParams.baudRate = 9600;
    uartParams.readMode = UART_MODE_BLOCKING;
    uartParams.writeMode = UART_MODE_BLOCKING;
    uartParams.readTimeout = UART_WAIT_FOREVER;
    uartParams.writeTimeout = UART_WAIT_FOREVER;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.parityType = UART_PAR_EVEN;

    uartUplink = UART_open(CONFIG_UART_0, &uartParams);
    uartDownlink = UART_open(CONFIG_UART_1, &uartParams);

    if ((uartUplink == NULL) || (uartDownlink == NULL)) {
        /* UART_open() failed, turn on user LED */
        GPIO_write(CONFIG_LED_0_GPIO, CONFIG_GPIO_LED_ON);
        while (1);
    }

#endif

#if 0
    bytesWritten = UART_write(uartUplink, echoPrompt, sizeof(echoPrompt));
    if (bytesWritten != sizeof(echoPrompt)) {
        /* UART_write() failed, turn on user LED */
        GPIO_write(CONFIG_LED_0_GPIO, CONFIG_GPIO_LED_ON);
        while (1);
    }
    bytesWritten = UART_write(uartDownlink, echoPrompt, sizeof(echoPrompt));
    if (bytesWritten != sizeof(echoPrompt)) {
        /* UART_write() failed, turn on user LED */
        GPIO_write(CONFIG_LED_0_GPIO, CONFIG_GPIO_LED_ON);
        while (1);
    }


    create_task(uartEcho, &taskUplink_args, "upLinkTh", tskIDLE_PRIORITY);
    create_task(uartEcho, &taskDownlink_args, "downLinkTh", tskIDLE_PRIORITY);
#endif

    LED_Params_init(&ledParams);
    ledRed = LED_open(CONFIG_LED_0, &ledParams);
    LED_open(CONFIG_LED_1, &ledParams);
    LED_open(CONFIG_LED_2, &ledParams);

    Button_Params_init(&buttonParams);
    buttonParams.debounceDuration = 10;
    buttonParams.longPressDuration = 2000;
    buttonParams.buttonEventMask = 0xFF;
    buttonParams.buttonCallback = &ButtonLeftCallback;
    buttonParams.buttonCallback = &ButtonRightCallback;

    buttonLeft = Button_open(CONFIG_BUTTON_0, &buttonParams);
    buttonRight = Button_open(CONFIG_BUTTON_1, &buttonParams);

    if ((buttonLeft == NULL) || (buttonRight == NULL)) {
        /* button_open() failed, turn on user LED */
        LED_startBlinking(ledRed, 250, LED_BLINK_FOREVER);
        while (1);
    }

    //knxLink_uart_t uarthandle1 = uartUplink;
    //knxLink_uart_t uarthandle2 = uartDownlink;

    uartUplink = knxLinkAdapterOpen(KNX_LINK_ADAPTER_UPLINK, KNX_LINK_ADAPTER_BPS_9600, KNX_LINK_ADAPTER_PARITY_EVEN);
    //uartDownlink = NULL;
    uartDownlink = knxLinkAdapterOpen(KNX_LINK_ADAPTER_DOWNLINK, KNX_LINK_ADAPTER_BPS_9600, KNX_LINK_ADAPTER_PARITY_EVEN);

    debugInit(uartDownlink);
    //debugPointer("mainThread, uartUplink = %p\r\n", uartUplink);

    knxlink_handle1 = knxLinkInit(MY_IA_ADDRESS, uartUplink);
    knxlink_handle2 = NULL;
    //knxlink_handle2 = knxLinkInit(MY_IA_ADDRESS, uartDownlink);
    //debugPointer("mainThread, handle1 = %p\r\n", knxlink_handle1);
    comm_data.objects = getCommissioningObjects();
    comm_data.objects_num = getCommissioningObjectsNum();
    comm_data.gas = getCommissioningGAs();
    comm_data.gas_num = getCommissioningGAsNum();
    knxAppInit(MY_IA_ADDRESS, &comm_data, knxlink_handle1, knxlink_handle2);

    /* Loop forever doing nothing */
    while (1) {
        debug("mainThread gonna sleep\r\n");
        vTaskDelay(1000);

    }
}
