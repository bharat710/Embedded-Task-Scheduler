/**
 * @file timer.h
 * @brief Software Timer for the Embedded Task Scheduler
 *
 * Provides a monotonic software timer that advances by a fixed
 * tick interval on each call to sw_timer_tick().
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief Software timer structure.
 */
typedef struct {
  uint32_t current_time_ms;  /**< Current system time in ms          */
  uint32_t tick_interval_ms; /**< Time advance per tick in ms        */
  uint64_t total_ticks;      /**< Total number of ticks elapsed      */
} sw_timer_t;

/**
 * @brief Initialise the software timer.
 * @param timer           Pointer to timer instance
 * @param tick_interval   Tick interval in milliseconds
 */
void sw_timer_init(sw_timer_t *timer, uint32_t tick_interval);

/**
 * @brief Advance the timer by one tick.
 * @param timer  Pointer to timer instance
 */
void sw_timer_tick(sw_timer_t *timer);

/**
 * @brief Get the current system time.
 * @param timer  Pointer to timer instance
 * @return       Current time in milliseconds
 */
uint32_t sw_timer_get_time(const sw_timer_t *timer);

/**
 * @brief Simulate a real-time delay for one tick.
 *
 * Calls usleep() to block for tick_interval_ms milliseconds,
 * providing a wall-clock-aligned simulation pace.
 *
 * @param timer  Pointer to timer instance
 */
void sw_timer_delay_tick(const sw_timer_t *timer);

#endif /* TIMER_H */
