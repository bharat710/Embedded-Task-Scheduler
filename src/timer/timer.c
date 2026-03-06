/**
 * @file timer.c
 * @brief Software Timer Implementation
 *
 * Provides a monotonic timer driven by usleep() to simulate
 * hardware timer ticks on a desktop/POSIX platform.
 */

#include "timer.h"
#include <stdio.h>
#include <unistd.h> /* usleep() */

void sw_timer_init(sw_timer_t *timer, uint32_t tick_interval) {
  if (!timer)
    return;

  timer->current_time_ms = 0;
  timer->tick_interval_ms = tick_interval;
  timer->total_ticks = 0;
}

void sw_timer_tick(sw_timer_t *timer) {
  if (!timer)
    return;

  timer->current_time_ms += timer->tick_interval_ms;
  timer->total_ticks++;
}

uint32_t sw_timer_get_time(const sw_timer_t *timer) {
  return timer ? timer->current_time_ms : 0;
}

void sw_timer_delay_tick(const sw_timer_t *timer) {
  if (!timer)
    return;

  /* Convert ms → µs for usleep */
  usleep(timer->tick_interval_ms * 1000u);
}
