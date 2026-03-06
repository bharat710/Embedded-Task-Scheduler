/**
 * @file scheduler.c
 * @brief Embedded Task Scheduler — Core Implementation
 *
 * Implements task management, Round Robin and Priority-Based scheduling,
 * periodic task release, and the main scheduler loop.
 */

#include "scheduler.h"
#include "../timer/timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────
 *  ANSI colour codes for pretty terminal output
 * ───────────────────────────────────────────── */
#define CLR_RESET "\033[0m"
#define CLR_GREEN "\033[1;32m"
#define CLR_YELLOW "\033[1;33m"
#define CLR_CYAN "\033[1;36m"
#define CLR_RED "\033[1;31m"
#define CLR_MAGENTA "\033[1;35m"
#define CLR_BLUE "\033[1;34m"
#define CLR_DIM "\033[2m"
#define CLR_BOLD "\033[1m"

/* ─────────────────────────────────────────────
 *  Internal: next unique task ID generator
 * ───────────────────────────────────────────── */
static uint32_t s_next_task_id = 1;

/* ─────────────────────────────────────────────
 *  Internal: scheduling algorithm helpers
 * ───────────────────────────────────────────── */

/**
 * @brief Select the next task using Round Robin.
 *
 * Iterates circularly through the task table, returning the
 * index of the first READY task found after current_index.
 *
 * @return  Task index, or -1 if no READY task exists.
 */
static int schedule_round_robin(scheduler_t *sched) {
  if (sched->task_count == 0)
    return -1;

  uint32_t start = (sched->current_index + 1) % MAX_TASKS;
  uint32_t i = start;

  do {
    task_control_block_t *tcb = &sched->tasks[i];
    if (tcb->active && tcb->state == TASK_STATE_READY) {
      return (int)i;
    }
    i = (i + 1) % MAX_TASKS;
  } while (i != start);

  return -1;
}

/**
 * @brief Select the next task using Priority-Based scheduling.
 *
 * Scans all tasks and selects the READY task with the lowest
 * priority value (0 = highest priority).  Ties are broken by
 * task ID (lower ID runs first).
 *
 * @return  Task index, or -1 if no READY task exists.
 */
static int schedule_priority(scheduler_t *sched) {
  int best_idx = -1;
  uint8_t best_priority = UINT8_MAX;
  uint32_t best_id = UINT32_MAX;

  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    task_control_block_t *tcb = &sched->tasks[i];
    if (!tcb->active || tcb->state != TASK_STATE_READY)
      continue;

    if (tcb->priority < best_priority ||
        (tcb->priority == best_priority && tcb->task_id < best_id)) {
      best_idx = (int)i;
      best_priority = tcb->priority;
      best_id = tcb->task_id;
    }
  }

  return best_idx;
}

/**
 * @brief Release periodic tasks whose next_release_ms <= system_time.
 *
 * Moves tasks from WAITING/SUSPENDED → READY when their period
 * has elapsed and updates their next release time.
 */
static void release_periodic_tasks(scheduler_t *sched) {
  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    task_control_block_t *tcb = &sched->tasks[i];
    if (!tcb->active)
      continue;
    /* Skip suspended tasks — they stay suspended until explicitly resumed */
    if (tcb->state == TASK_STATE_SUSPENDED)
      continue;

    if (sched->system_time_ms >= tcb->next_release_ms) {
      tcb->state = TASK_STATE_READY;
      tcb->next_release_ms = sched->system_time_ms + tcb->period_ms;
      tcb->elapsed_ms = 0;
    }
  }
}

/* ─────────────────────────────────────────────
 *  Public API — Lifecycle
 * ───────────────────────────────────────────── */

void scheduler_init(scheduler_t *sched, uint32_t tick_interval,
                    uint32_t max_ticks, sched_policy_t policy) {
  if (!sched)
    return;

  memset(sched, 0, sizeof(scheduler_t));
  sched->tick_interval_ms = tick_interval;
  sched->max_ticks = max_ticks;
  sched->policy = policy;
  sched->running = false;
  sched->current_index = MAX_TASKS - 1; /* so first RR pick starts at 0 */
  s_next_task_id = 1;

  printf("\n%s╔══════════════════════════════════════════════════╗%s\n",
         CLR_CYAN, CLR_RESET);
  printf("%s║   EMBEDDED TASK SCHEDULER — Initialised          ║%s\n", CLR_CYAN,
         CLR_RESET);
  printf("%s╚══════════════════════════════════════════════════╝%s\n", CLR_CYAN,
         CLR_RESET);
  printf("  Policy       : %s%s%s\n", CLR_GREEN, sched_policy_to_string(policy),
         CLR_RESET);
  printf("  Tick interval: %s%u ms%s\n", CLR_GREEN, tick_interval, CLR_RESET);
  printf("  Max ticks    : %s%u%s\n\n", CLR_GREEN, max_ticks, CLR_RESET);
}

void scheduler_run(scheduler_t *sched) {
  if (!sched)
    return;

  sw_timer_t timer;
  sw_timer_init(&timer, sched->tick_interval_ms);
  sched->running = true;

  printf("%s── Scheduler started ──────────────────────────────%s\n\n",
         CLR_BOLD, CLR_RESET);

  uint32_t tick = 0;

  while (sched->running) {
    /* --- Time Update --- */
    sched->system_time_ms = sw_timer_get_time(&timer);

    /* --- Release periodic tasks --- */
    release_periodic_tasks(sched);

    /* --- Select next task --- */
    int idx = -1;
    switch (sched->policy) {
    case SCHED_ROUND_ROBIN:
      idx = schedule_round_robin(sched);
      break;
    case SCHED_PRIORITY:
      idx = schedule_priority(sched);
      break;
    }

    if (idx >= 0) {
      task_control_block_t *tcb = &sched->tasks[idx];

      /* Transition previous running task back to READY */
      for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (sched->tasks[i].active &&
            sched->tasks[i].state == TASK_STATE_RUNNING) {
          sched->tasks[i].state = TASK_STATE_READY;
        }
      }

      /* Run selected task */
      tcb->state = TASK_STATE_RUNNING;
      sched->current_index = (uint32_t)idx;

      printf("%s[%4u ms]%s  ▶ %s%-18s%s  (prio=%u  state=%s%s%s)\n", CLR_DIM,
             sched->system_time_ms, CLR_RESET, CLR_GREEN, tcb->name, CLR_RESET,
             tcb->priority, CLR_YELLOW, task_state_to_string(tcb->state),
             CLR_RESET);

      /* Execute the task function */
      if (tcb->execute) {
        tcb->execute(tcb->task_id);
      }

      tcb->execution_count++;
      tcb->elapsed_ms += sched->tick_interval_ms;

      /* After execution, move to WAITING until next period release */
      tcb->state = TASK_STATE_WAITING;

    } else {
      printf("%s[%4u ms]%s  %s— IDLE (no ready tasks) —%s\n", CLR_DIM,
             sched->system_time_ms, CLR_RESET, CLR_RED, CLR_RESET);
    }

    /* --- Advance timer --- */
    sw_timer_tick(&timer);

    /* --- Real-time pacing --- */
    sw_timer_delay_tick(&timer);

    /* --- Check termination --- */
    tick++;
    if (sched->max_ticks > 0 && tick >= sched->max_ticks) {
      sched->running = false;
    }
  }

  printf("\n%s── Scheduler stopped at %u ms ─────────────────────%s\n\n",
         CLR_BOLD, sched->system_time_ms, CLR_RESET);
}

void scheduler_stop(scheduler_t *sched) {
  if (sched)
    sched->running = false;
}

/* ─────────────────────────────────────────────
 *  Public API — Task Management
 * ───────────────────────────────────────────── */

int scheduler_add_task(scheduler_t *sched, const char *name, uint8_t priority,
                       uint32_t period_ms, task_func_t func) {
  if (!sched || !func)
    return -1;

  /* Find a free slot */
  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    if (!sched->tasks[i].active) {
      task_control_block_t *tcb = &sched->tasks[i];
      memset(tcb, 0, sizeof(task_control_block_t));

      tcb->task_id = s_next_task_id++;
      tcb->priority = priority;
      tcb->state = TASK_STATE_READY;
      tcb->period_ms = period_ms;
      tcb->deadline_ms = period_ms; /* default: deadline = period */
      tcb->next_release_ms = 0;     /* first release at time 0    */
      tcb->execute = func;
      tcb->active = true;

      strncpy(tcb->name, name ? name : "unnamed", TASK_NAME_MAX_LEN - 1);

      sched->task_count++;

      printf("  %s+ Task added:%s  ID=%u  name=%-18s  prio=%u  "
             "period=%u ms\n",
             CLR_CYAN, CLR_RESET, tcb->task_id, tcb->name, priority, period_ms);

      return (int)tcb->task_id;
    }
  }

  fprintf(stderr, "%s[ERROR] Task table full (max %d)%s\n", CLR_RED, MAX_TASKS,
          CLR_RESET);
  return -1;
}

int scheduler_remove_task(scheduler_t *sched, uint32_t task_id) {
  if (!sched)
    return -1;

  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    if (sched->tasks[i].active && sched->tasks[i].task_id == task_id) {
      printf("  %s- Task removed:%s ID=%u  name=%s\n", CLR_RED, CLR_RESET,
             task_id, sched->tasks[i].name);
      sched->tasks[i].active = false;
      sched->task_count--;
      return 0;
    }
  }
  return -1;
}

int scheduler_suspend_task(scheduler_t *sched, uint32_t task_id) {
  if (!sched)
    return -1;

  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    task_control_block_t *tcb = &sched->tasks[i];
    if (tcb->active && tcb->task_id == task_id) {
      tcb->state = TASK_STATE_SUSPENDED;
      printf("  %s⏸ Task suspended:%s ID=%u  name=%s\n", CLR_YELLOW, CLR_RESET,
             task_id, tcb->name);
      return 0;
    }
  }
  return -1;
}

int scheduler_resume_task(scheduler_t *sched, uint32_t task_id) {
  if (!sched)
    return -1;

  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    task_control_block_t *tcb = &sched->tasks[i];
    if (tcb->active && tcb->task_id == task_id &&
        tcb->state == TASK_STATE_SUSPENDED) {
      tcb->state = TASK_STATE_READY;
      printf("  %s▶ Task resumed:%s  ID=%u  name=%s\n", CLR_GREEN, CLR_RESET,
             task_id, tcb->name);
      return 0;
    }
  }
  return -1;
}

/* ─────────────────────────────────────────────
 *  Public API — Status & Debugging
 * ───────────────────────────────────────────── */

void scheduler_print_status(const scheduler_t *sched) {
  if (!sched)
    return;

  printf(
      "\n%s┌─────┬────────────────────┬──────┬───────────┬───────────┬───────┐%"
      "s\n",
      CLR_CYAN, CLR_RESET);
  printf("%s│ ID  │ Name               │ Prio │ State     │ Period ms │ Runs  "
         "│%s\n",
         CLR_CYAN, CLR_RESET);
  printf("%s├─────┼────────────────────┼──────┼───────────┼───────────┼───────┤"
         "%s\n",
         CLR_CYAN, CLR_RESET);

  for (uint32_t i = 0; i < MAX_TASKS; i++) {
    const task_control_block_t *tcb = &sched->tasks[i];
    if (!tcb->active)
      continue;

    const char *state_clr = CLR_RESET;
    switch (tcb->state) {
    case TASK_STATE_RUNNING:
      state_clr = CLR_GREEN;
      break;
    case TASK_STATE_READY:
      state_clr = CLR_YELLOW;
      break;
    case TASK_STATE_WAITING:
      state_clr = CLR_BLUE;
      break;
    case TASK_STATE_SUSPENDED:
      state_clr = CLR_RED;
      break;
    }

    printf("│ %s%3u%s │ %-18s │  %3u │ %s%-9s%s │   %5u   │ %5u │\n", CLR_BOLD,
           tcb->task_id, CLR_RESET, tcb->name, tcb->priority, state_clr,
           task_state_to_string(tcb->state), CLR_RESET, tcb->period_ms,
           tcb->execution_count);
  }

  printf("%s└─────┴────────────────────┴──────┴───────────┴───────────┴───────┘"
         "%s\n\n",
         CLR_CYAN, CLR_RESET);
}

const char *task_state_to_string(task_state_t state) {
  switch (state) {
  case TASK_STATE_READY:
    return "READY";
  case TASK_STATE_RUNNING:
    return "RUNNING";
  case TASK_STATE_WAITING:
    return "WAITING";
  case TASK_STATE_SUSPENDED:
    return "SUSPENDED";
  default:
    return "UNKNOWN";
  }
}

const char *sched_policy_to_string(sched_policy_t policy) {
  switch (policy) {
  case SCHED_ROUND_ROBIN:
    return "Round Robin";
  case SCHED_PRIORITY:
    return "Priority-Based";
  default:
    return "Unknown";
  }
}
