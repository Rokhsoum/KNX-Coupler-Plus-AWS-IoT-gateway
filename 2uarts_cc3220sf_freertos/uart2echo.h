/*
 * uart2echo.h
 *
 *  Created on: 20 Jun 2022
 *      Author: prodr
 */

#ifndef UART2ECHO_H_
#define UART2ECHO_H_

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>


/* Stack size in bytes */
#define THREADSTACKSIZE   1024

TaskHandle_t create_task(TaskFunction_t func, void *arg, char *taskname, int priority);

void mainThread(void *arg0);


#endif /* UART2ECHO_H_ */
