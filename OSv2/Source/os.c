/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Operating Sytem 									               	   	     */
/**
 *	@file		os.c
 *	@brief		This file contains the sample test program for OS debugging.
 *	@details 	In this test, two tasks which separately controls an LED will
 *				be executed if their corresponding pushbutton is pressed
 *				causing the LED to toggle based on their scheduled time or
 *				execution interval, respectively. Each task requires three
 *				callback functions: initialization, execution, and wait.
 *				Commonly, initialization function is run once but ahead of
 *				the execution function while the execution function is run
 *				indefinitely but only after the initialization function is
 *				run first. However, if the wait function is present, the
 *				execution function will be under the wait function's control.
 *				If the wait function returns a logic HIGH (or TRUE) value, the
 *				execution function will not run regardless of the scheduled
 *				interval. Conversely, if the wait function returns a logic LOW
 *				(or FALSE) value, the  execution function will run according to
 *				its scheduled interval. In the absence of either initialization
 *				or execution function, the task will not run. Also, each task
 *				requires a task property which can only be specified once (not
 *				flexible to change period, priority, or the pointed callback
 *				functions). All tasks are added and sequenced by the task
 *				scheduler. The task scheduler must initialize all the tasks
 *				first before running the execution code.
**/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *	Copyright (C) 2015-2017 Xeno Xerxes Masong (xxmasong@gmail.com)
 *
 *	This program does not guarantee that it shall fulfill its task
 *	under all circumstances. Thus, the author/developer shall not
 *	be held liable for any damages that might be incurred by the
 *	device using this program, or for any reason whatsoever.
 *
 *	This program is free software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, either version 3 of
 *	the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <https://www.gnu.org/licenses/>.
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *	Target Hardware Information:
 *
 *		MCU			: F2MC-16LX MB90385 Series
 *		CPU			: MB90F387S
 *		Vcc			: 5.0 V
 *		Frequency	: 16 MHz (4 MHz PLLx4)
 *		Inputs		: P25, P27
 *		Outputs		: P10-P11
 *		Driver		: N/A
 *		Helper		: DIO, TBT (internal)
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <GPIO/dio.h>
#include <TMR/tbt.h>

#include <OS/timer.h>
#include <OS/scheduler.h>

/* ----------------------------------------------------------------------------
**	Test Task Prototypes.
*/

TASK(task10Init);
TASK(task10Exec);
TASK_WAIT(task10Wait);

TASK(task11Init);
TASK(task11Exec);
TASK_WAIT(task11Wait);

/* ----------------------------------------------------------------------------
**	Test Constants.
*/

/**
 *	@var 		taskProp1
 *	@brief		Task properties specified for port 1.
 */
static const T_taskProp taskProp1[] = {
	{
		(T_taskID)1U,
		(T_taskName)"Task-10",
		(T_taskPrio)TASK_PRIO(1U),
		(T_taskTime)TASK_SCHED(0UL, 0UL, 500UL),			/* TODO:EXPERIMENT: Change task10's interval (min, sec, millisec)! */
		(T_taskCBRun)&task10Init,
		(T_taskCBRun)&task10Exec,
		(T_taskCBWait)&task10Wait
	}, {
		(T_taskID)2U,
		(T_taskName)"Task-11",
		(T_taskPrio)TASK_PRIO(1U),
		(T_taskTime)TASK_SCHED(0UL, 1UL, 0UL),				/* TODO:EXPERIMENT: Change task11's interval (min, sec, millisec)! */
		(T_taskCBRun)&task11Init,
		(T_taskCBRun)&task11Exec,
		(T_taskCBWait)&task11Wait
	}
};

/* ----------------------------------------------------------------------------
**	Test Variables.
*/

/**
 *	@var 		taskManager
 *	@brief		Scheduler / task manager data.
 */
static T_scheduler taskManager;

/**
 *	@var 		task1
 *	@brief		Tasks of port 1.
 */
static T_task task1[2];

/* ----------------------------------------------------------------------------
**	Test Tasks.
*/

TASK(task10Init)
{
	(T_void)TASK_THIS;

	/* initialize pin mode and state */
	pinModeDigital(PIN10, PIN_OUTPUT);
	pinModeDigital(PIN25, PIN_INPUT);
	writeDigital(PIN10, PIN_HIGH);
}

TASK(task10Exec)
{
	(T_void)TASK_THIS;

	/* toggle LED */
	writeDigital(PIN10, NOT(readDigital(PIN10)));
}

TASK_WAIT(task10Wait)
{
	static T_swDebBuffer swDebPin25;
	static T_pinLevel wait25;

	(T_void)TASK_THIS;

	/* check button debounce state of pin 25 */
	if (NOT(readDebouncedNegTrig(&swDebPin25, 8U, ReadBitIOPinData(PIN_P25)))) {
		/* clear task wait */
		wait25 = FALSE;
	} else {
		/* set task wait */
		wait25 = TRUE;
	}

	return wait25;
}

TASK(task11Init)
{
	(T_void)TASK_THIS;

	/* initialize pin mode and state */
	pinModeDigital(PIN11, PIN_OUTPUT);
	pinModeDigital(PIN27, PIN_INPUT);
	writeDigital(PIN11, PIN_HIGH);
}

TASK(task11Exec)
{
	(T_void)TASK_THIS;

	/* toggle LED */
	writeDigital(PIN11, NOT(readDigital(PIN11)));
}

TASK_WAIT(task11Wait)
{
	static T_swDebBuffer swDebPin27;
	static T_pinLevel wait27;

	(T_void)TASK_THIS;

	/* check button debounce state of pin 27 */
	if (NOT(readDebouncedNegTrig(&swDebPin27, 8U, ReadBitIOPinData(PIN_P27)))) {
		/* clear task wait */
		wait27 = FALSE;
	} else {
		/* set task wait */
		wait27 = TRUE;
	}

	return wait27;
}

/* ----------------------------------------------------------------------------
**	Inline Functions.
*/

/**
 *  @fn			T_void init(T_void)
 *  @brief 		Initialization / setup code runs once.
 *  @param		.
 *	@return		.
 */
#pragma inline init
static T_void init(T_void)
{
	__DI();
	__set_il(7);

	/* initialize OS timer */
	initTBT();

	/* initialize scheduler */
	initScheduler((T_scheduler*)memset(&taskManager, NULL, SzBytes_(T_scheduler)));
	/* add all initialized tasks to scheduler */
	addSchedulerTask(&taskManager, initTask((T_task*)memset(&task1[0], NULL, SzBytes_(T_task)), &taskProp1[0], NULL));
	addSchedulerTask(&taskManager, initTask((T_task*)memset(&task1[1], NULL, SzBytes_(T_task)), &taskProp1[1], NULL));
	/* run initialization function from scheduler for all tasks */
	runSchedulerInit(&taskManager);

	__EI();
}

/**
 *  @fn			T_void debug(T_void)
 *  @brief 		Iteration / loop code runs repeatedly.
 *  @param		.
 *	@return		.
 */
#pragma inline debug
static T_void debug(T_void)
{
	/* run execution function from scheduler for all tasks */
	runSchedulerExec(&taskManager);
}

/* ----------------------------------------------------------------------------
**	Main Function.
*/

/**
 *  @fn			T_void main(T_void)
 *  @brief 		Main / program entry point.
 *  @param		.
 *	@return		.
 */
T_void FAR main(T_void)
{
	/* initialize first */
	init();
	/* then iterate indefinitely */
	for (;;) {
		debug();
	}
}

/* END OF OS. */
