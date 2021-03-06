/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Timer objects and scheduling management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "type.h"
#include "rtc.h"

/*!
 * \brief Timer object description
 */
typedef struct TimerEvent_s
{
    uint32_t Timestamp;         //! Current timer value
    uint32_t ReloadValue;       //! Timer delay value
    bool IsRunning;             //! Is the timer currently running
    void ( *Callback )( void ); //! Timer IRQ callback function
    char *name;                 //! Timer name
    struct TimerEvent_s *Next;  //! Pointer to the next Timer object.
}TimerEvent_t;

/*!
 * \brief Initializes the timer object
 *
 * \remark TimerSetValue function must be called before starting the timer.
 *         this function initializes timestamp and reload value at 0.
 *
 * \param [IN] obj          Pointer to the timer object parameters
 * \param [IN] callback     Function callback called at the end of the timeout
 * \param [IN] name         Pointer to the name of the timer object
 */
void TimerInit( TimerEvent_t *obj, void ( *callback )( void ), char *name );

/*!
 * Timer IRQ event handler
 */
void TimerIrqHandler( void );

/*!
 * \brief Starts and adds the timer object to the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerStart( TimerEvent_t *obj );

/*!
 * \brief Stops and removes the timer object from the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerStop( TimerEvent_t *obj );

/*!
 * \brief Resets the timer object
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerReset( TimerEvent_t *obj );

/*!
 * \brief Set timer new timeout value
 *
 * \param [IN] obj      Structure containing the timer object parameters
 * \param [IN] value    New timer timeout value
 */
void TimerSetValue( TimerEvent_t *obj, uint32_t value );

/*!
 * \brief Read the current time
 *
 * \retval time Returns current time
 */
TimerTime_t TimerGetCurrentTime( void );

/*!
 * \brief Get the elapsed time since the saved time
 *
 * \retval time Returns elapsed time
 */
uint32_t TimerGetElapsedTime( TimerTime_t savedTime );

#endif  // __TIMER_H__
