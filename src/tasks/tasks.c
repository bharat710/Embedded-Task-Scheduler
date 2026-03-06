/**
 * @file tasks.c
 * @brief Example Embedded Task Implementations
 *
 * Each task simulates a typical embedded workload and prints
 * a short log line to stdout so scheduling behaviour is visible.
 */

#include "tasks.h"
#include <stdio.h>
#include <stdlib.h>

/* ANSI colours for per-task identification */
#define CLR_RESET "\033[0m"
#define CLR_GREEN "\033[0;32m"
#define CLR_YELLOW "\033[0;33m"
#define CLR_BLUE "\033[0;34m"
#define CLR_MAGENTA "\033[0;35m"
#define CLR_CYAN "\033[0;36m"

/* ── LED Toggle ─────────────────────────────── */

static int led_state = 0;

void task_led_toggle(uint32_t task_id) {
  led_state = !led_state;
  printf("           %s💡 [Task %u] LED → %s%s\n", CLR_GREEN, task_id,
         led_state ? "ON " : "OFF", CLR_RESET);
}

/* ── Sensor Read ────────────────────────────── */

void task_sensor_read(uint32_t task_id) {
  /* Simulate a temperature reading between 20.0°C and 35.0°C */
  float temp = 20.0f + ((float)(rand() % 150) / 10.0f);
  printf("           %s🌡  [Task %u] Sensor temp = %.1f °C%s\n", CLR_CYAN,
         task_id, temp, CLR_RESET);
}

/* ── Data Logging ───────────────────────────── */

static uint32_t log_seq = 0;

void task_data_logging(uint32_t task_id) {
  log_seq++;
  printf("           %s📝 [Task %u] Log entry #%u written%s\n", CLR_YELLOW,
         task_id, log_seq, CLR_RESET);
}

/* ── Motor Control ──────────────────────────── */

void task_motor_control(uint32_t task_id) {
  uint8_t duty = (uint8_t)(rand() % 101); /* 0 – 100 % */
  printf("           %s⚙️  [Task %u] Motor PWM duty = %u%%%s\n", CLR_MAGENTA,
         task_id, duty, CLR_RESET);
}

/* ── Communication ──────────────────────────── */

static uint32_t pkt_seq = 0;

void task_communication(uint32_t task_id) {
  pkt_seq++;
  printf("           %s📡 [Task %u] Packet #%u sent%s\n", CLR_BLUE, task_id,
         pkt_seq, CLR_RESET);
}
