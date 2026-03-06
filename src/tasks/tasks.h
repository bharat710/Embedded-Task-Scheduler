/**
 * @file tasks.h
 * @brief Example embedded tasks for demonstration
 *
 * Each function simulates a typical embedded workload
 * (LED, sensor, logging, motor, communication).
 */

#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>

/**
 * @brief Simulate toggling an LED on/off.
 * @param task_id  The calling task's ID
 */
void task_led_toggle(uint32_t task_id);

/**
 * @brief Simulate reading a sensor value.
 * @param task_id  The calling task's ID
 */
void task_sensor_read(uint32_t task_id);

/**
 * @brief Simulate logging data to storage.
 * @param task_id  The calling task's ID
 */
void task_data_logging(uint32_t task_id);

/**
 * @brief Simulate adjusting motor PWM duty cycle.
 * @param task_id  The calling task's ID
 */
void task_motor_control(uint32_t task_id);

/**
 * @brief Simulate sending a data packet over a bus.
 * @param task_id  The calling task's ID
 */
void task_communication(uint32_t task_id);

#endif /* TASKS_H */
