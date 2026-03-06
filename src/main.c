/**
 * @file main.c
 * @brief Embedded Task Scheduler — Entry Point
 *
 * Registers five example tasks with different periods and priorities,
 * then runs the scheduler.
 *
 * Usage:
 *   ./build/scheduler [policy] [ticks]
 *
 *   policy  : "rr" (Round Robin, default) or "priority"
 *   ticks   : number of scheduler ticks to run (default 40)
 *
 * Example:
 *   ./build/scheduler priority 30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "scheduler/scheduler.h"
#include "tasks/tasks.h"

/* ─────────────────────────────────────────────
 *  ANSI helpers
 * ───────────────────────────────────────────── */
#define CLR_RESET "\033[0m"
#define CLR_BOLD "\033[1m"
#define CLR_CYAN "\033[1;36m"
#define CLR_GREEN "\033[1;32m"
#define CLR_YELLOW "\033[1;33m"
#define CLR_RED "\033[1;31m"

/* ─────────────────────────────────────────────
 *  Configuration defaults
 * ───────────────────────────────────────────── */
#define DEFAULT_TICK_MS 5
#define DEFAULT_MAX_TICKS 40

/* ─────────────────────────────────────────────
 *  Banner
 * ───────────────────────────────────────────── */
static void print_banner(void) {
  printf("\n");
  printf("%s", CLR_CYAN);
  printf("  ███████╗███╗   ███╗██████╗ ███████╗██████╗ ██████╗ ███████╗██████╗ "
         "\n");
  printf("  ██╔════╝████╗ "
         "████║██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔════╝██╔══██╗\n");
  printf("  █████╗  ██╔████╔██║██████╔╝█████╗  ██║  ██║██║  ██║█████╗  ██║  "
         "██║\n");
  printf("  ██╔══╝  ██║╚██╔╝██║██╔══██╗██╔══╝  ██║  ██║██║  ██║██╔══╝  ██║  "
         "██║\n");
  printf("  ███████╗██║ ╚═╝ "
         "██║██████╔╝███████╗██████╔╝██████╔╝███████╗██████╔╝\n");
  printf("  ╚══════╝╚═╝     ╚═╝╚═════╝ ╚══════╝╚═════╝ ╚═════╝ ╚══════╝╚═════╝ "
         "\n");
  printf("%s", CLR_RESET);
  printf("\n");
  printf(
      "  "
      "%s╔══════════════════════════════════════════════════════════════╗%s\n",
      CLR_CYAN, CLR_RESET);
  printf(
      "  %s║          EMBEDDED   TASK   SCHEDULER   v1.0                ║%s\n",
      CLR_CYAN, CLR_RESET);
  printf(
      "  %s║          POSIX Simulation  •  C11                          ║%s\n",
      CLR_CYAN, CLR_RESET);
  printf(
      "  "
      "%s╚══════════════════════════════════════════════════════════════╝%s\n",
      CLR_CYAN, CLR_RESET);
  printf("\n");
}

/* ─────────────────────────────────────────────
 *  Usage
 * ───────────────────────────────────────────── */
static void print_usage(const char *prog) {
  printf("Usage: %s [policy] [ticks]\n\n", prog);
  printf("  policy : %srr%s        Round Robin (default)\n", CLR_GREEN,
         CLR_RESET);
  printf("           %spriority%s  Priority-Based scheduling\n", CLR_GREEN,
         CLR_RESET);
  printf("  ticks  : number of scheduler ticks (default %d)\n\n",
         DEFAULT_MAX_TICKS);
  printf("Example:\n");
  printf("  %s ./build/scheduler priority 30%s\n\n", CLR_YELLOW, CLR_RESET);
}

/* ─────────────────────────────────────────────
 *  Main
 * ───────────────────────────────────────────── */
int main(int argc, char *argv[]) {
  sched_policy_t policy = SCHED_ROUND_ROBIN;
  uint32_t max_ticks = DEFAULT_MAX_TICKS;

  /* ── Parse CLI arguments ───────────────── */
  if (argc >= 2) {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_usage(argv[0]);
      return 0;
    }
    if (strcmp(argv[1], "priority") == 0) {
      policy = SCHED_PRIORITY;
    } else if (strcmp(argv[1], "rr") == 0) {
      policy = SCHED_ROUND_ROBIN;
    } else {
      fprintf(stderr,
              "%s[ERROR] Unknown policy '%s'. "
              "Use 'rr' or 'priority'.%s\n",
              CLR_RED, argv[1], CLR_RESET);
      return 1;
    }
  }
  if (argc >= 3) {
    max_ticks = (uint32_t)atoi(argv[2]);
    if (max_ticks == 0)
      max_ticks = DEFAULT_MAX_TICKS;
  }

  /* Seed random for sensor / motor simulation */
  srand((unsigned)time(NULL));

  print_banner();

  /* ── Initialise scheduler ──────────────── */
  scheduler_t sched;
  scheduler_init(&sched, DEFAULT_TICK_MS, max_ticks, policy);

  /* ── Register example tasks ────────────── */
  /*
   * Task Name           Priority   Period (ms)   Description
   * ─────────────────────────────────────────────────────────
   * LED Toggle           2          10            Blinks LED every 10 ms
   * Sensor Read          1          20            Reads sensor every 20 ms
   * Data Logger          3          15            Logs data every 15 ms
   * Motor Control        0          25            Adjusts motor every 25 ms
   * Communication        4          30            Sends packet every 30 ms
   */

  printf("\n%s── Registering Tasks ─────────────────────────────%s\n\n",
         CLR_BOLD, CLR_RESET);

  scheduler_add_task(&sched, "LED Toggle", 2, 10, task_led_toggle);
  scheduler_add_task(&sched, "Sensor Read", 1, 20, task_sensor_read);
  scheduler_add_task(&sched, "Data Logger", 3, 15, task_data_logging);
  scheduler_add_task(&sched, "Motor Control", 0, 25, task_motor_control);
  scheduler_add_task(&sched, "Communication", 4, 30, task_communication);

  /* ── Print initial task table ──────────── */
  printf("\n%s── Initial Task Table ────────────────────────────%s\n", CLR_BOLD,
         CLR_RESET);
  scheduler_print_status(&sched);

  /* ── Run scheduler ─────────────────────── */
  scheduler_run(&sched);

  /* ── Print final summary ───────────────── */
  printf("%s── Final Task Summary ────────────────────────────%s\n", CLR_BOLD,
         CLR_RESET);
  scheduler_print_status(&sched);

  printf("%s✓ Scheduler exited normally.%s\n\n", CLR_GREEN, CLR_RESET);

  return 0;
}
