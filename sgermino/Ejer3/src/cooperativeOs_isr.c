/* Copyright 2013, Michael J. Pont.
 * Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 2016-08-16 */


#include "cooperativeOs_isr.h"
#include "cooperativeOs_scheduler.h"
#include "board.h"


extern void (* g_tickHook)();

/* The array of tasks (see Sch.C) */
extern sTask_t schedulerTasks[SCHEDULER_MAX_TASKS];

/* The error code variable */
extern uint8_t errorCode;


/*------------------------------------------------------------------*
schedulerInit()
Scheduler initialization function. Prepares scheduler
data structures.
Must call this function before using the scheduler.
-*------------------------------------------------------------------*/
void schedulerInit (void)
{
    for (uint32_t i = 0; i < SCHEDULER_MAX_TASKS; ++i)
    {
        schedulerDeleteTask(i);
    }

   /* Reset the global error variable
      - schedulerDelete_Task() will generate an error code,
        (because the task array is empty) */
    errorCode = 0;

   // serialDebugPrintLnString( "Tick configurado a 1ms." );
}

/*------------------------------------------------------------------*
schedulerUpdate()
This is the scheduler ISR. It is called at a rate
determined by the timer settings in the 'init' function.
This version is triggered by Timer 2 interrupts:
timer is automatically reloaded.
-*------------------------------------------------------------------*/
void schedulerUpdate ()
{
    // NOTE: calculations are in *TICKS* (not milliseconds)
    for (uint32_t i = 0; i < SCHEDULER_MAX_TASKS; ++i)
    {
        sTask_t* t = &schedulerTasks[i];
        // Check if there is a task at this location
        if (!t->pTask || -- t->delay > 0)
        {
            continue;
        }

        // The task is due to run
        ++ t->runMe;
        if (t->period)
        {
            // Schedule regular tasks to run again
            t->delay = t->period;
        }
    }
}

/*------------------------------------------------------------------*
schedulerStart()
Starts the scheduler, by enabling timer interrupts.
NOTE: Usually called after all regular tasks are added,
to keep the tasks synchronized.
NOTE: ONLY THE SCHEDULER INTERRUPT SHOULD BE ENABLED!!!
-*------------------------------------------------------------------*/
void schedulerStart (uint32_t tickRateMs)
{
   /* Inicializar el conteo de Ticks con resolucion de tickRateMs ms (se
      ejecuta periódicamente una interrupcón cada tickRateMs ms que
      incrementa un contador de Ticks obteniendose una base de tiempos).
      Se agrega además como "tick hook" a la funcion encargada de planificar
      las tareas seosScheduleTasks().
      El tick hook es simplemente una función que se ejecutará períodicamente
      con cada interrupción de Tick, este nombre se refiere a una función
      "enganchada" a una interrupción */

    SysTick_Config (SystemCoreClock / (tickRateMs * 1000));
    g_tickHook = schedulerUpdate;
}