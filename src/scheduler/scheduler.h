/**
 * @file scheduler.h
 * @brief Embedded Task Scheduler - Core Header
 *
 * Defines the Task Control Block (TCB), scheduler structures,
 * scheduling policies, and public API for task management and scheduling.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────
 *  Configuration
 * ───────────────────────────────────────────── */

/** Maximum number of tasks the scheduler can manage */
#define MAX_TASKS           16

/** Maximum length of a task name string */
#define TASK_NAME_MAX_LEN   32

/* ─────────────────────────────────────────────
 *  Task State Enumeration
 * ───────────────────────────────────────────── */

/**
 * @brief Task states managed by the scheduler.
 *
 * State transitions:
 *   READY ──► RUNNING  (scheduler selects this task)
 *   RUNNING ──► READY  (time slice expires / preempted)
 *   RUNNING ──► WAITING (task blocked on resource)
 *   WAITING ──► READY  (resource released)
 *   Any ──► SUSPENDED  (explicitly suspended)
 *   SUSPENDED ──► READY (explicitly resumed)
 */
typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_WAITING,
    TASK_STATE_SUSPENDED
} task_state_t;

/* ─────────────────────────────────────────────
 *  Scheduling Policy Enumeration
 * ───────────────────────────────────────────── */

typedef enum {
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY
} sched_policy_t;

/* ─────────────────────────────────────────────
 *  Task Control Block (TCB)
 * ───────────────────────────────────────────── */

/** Task function signature – takes the task's own ID as argument */
typedef void (*task_func_t)(uint32_t task_id);

/**
 * @brief Task Control Block — stores all metadata for a single task.
 */
typedef struct {
    uint32_t      task_id;                      /**< Unique task identifier          */
    char          name[TASK_NAME_MAX_LEN];      /**< Human-readable task name        */
    uint8_t       priority;                     /**< Priority (0 = highest)          */
    task_state_t  state;                        /**< Current execution state         */
    uint32_t      period_ms;                    /**< Execution period in ms          */
    uint32_t      deadline_ms;                  /**< Relative deadline in ms         */
    uint32_t      elapsed_ms;                   /**< Time elapsed since last release */
    uint32_t      next_release_ms;              /**< Next scheduled release time     */
    uint32_t      execution_count;              /**< Number of times task has run    */
    task_func_t   execute;                      /**< Pointer to task function        */
    bool          active;                       /**< Slot in use flag                */
} task_control_block_t;

/* ─────────────────────────────────────────────
 *  Scheduler Structure
 * ───────────────────────────────────────────── */

/**
 * @brief Core scheduler structure.
 */
typedef struct {
    task_control_block_t  tasks[MAX_TASKS];     /**< Task table (fixed array)        */
    uint32_t              task_count;           /**< Number of active tasks          */
    uint32_t              current_index;        /**< Last-scheduled task index (RR)  */
    uint32_t              system_time_ms;       /**< Monotonic system time in ms     */
    uint32_t              tick_interval_ms;     /**< Duration of one scheduler tick  */
    uint32_t              max_ticks;            /**< Total ticks before shutdown     */
    sched_policy_t        policy;              /**< Active scheduling policy        */
    bool                  running;             /**< Scheduler running flag          */
} scheduler_t;

/* ─────────────────────────────────────────────
 *  Public API — Scheduler Lifecycle
 * ───────────────────────────────────────────── */

/**
 * @brief Initialise the scheduler.
 * @param sched           Pointer to scheduler instance
 * @param tick_interval   Tick interval in milliseconds
 * @param max_ticks       Number of ticks to run (0 = infinite)
 * @param policy          Scheduling policy to use
 */
void scheduler_init(scheduler_t *sched, uint32_t tick_interval,
                    uint32_t max_ticks, sched_policy_t policy);

/**
 * @brief Run the scheduler main loop.
 * @param sched  Pointer to scheduler instance
 */
void scheduler_run(scheduler_t *sched);

/**
 * @brief Stop the scheduler.
 * @param sched  Pointer to scheduler instance
 */
void scheduler_stop(scheduler_t *sched);

/* ─────────────────────────────────────────────
 *  Public API — Task Management
 * ───────────────────────────────────────────── */

/**
 * @brief Create and register a new task.
 * @return  Task ID on success, -1 on failure
 */
int scheduler_add_task(scheduler_t *sched, const char *name,
                       uint8_t priority, uint32_t period_ms,
                       task_func_t func);

/**
 * @brief Remove a task by ID.
 * @return  0 on success, -1 if not found
 */
int scheduler_remove_task(scheduler_t *sched, uint32_t task_id);

/**
 * @brief Suspend a task (set state to SUSPENDED).
 * @return  0 on success, -1 if not found
 */
int scheduler_suspend_task(scheduler_t *sched, uint32_t task_id);

/**
 * @brief Resume a suspended task (set state to READY).
 * @return  0 on success, -1 if not found / not suspended
 */
int scheduler_resume_task(scheduler_t *sched, uint32_t task_id);

/* ─────────────────────────────────────────────
 *  Public API — Status & Debugging
 * ───────────────────────────────────────────── */

/**
 * @brief Print the current task table to stdout.
 */
void scheduler_print_status(const scheduler_t *sched);

/**
 * @brief Get a human-readable string for a task state.
 */
const char *task_state_to_string(task_state_t state);

/**
 * @brief Get a human-readable string for a scheduling policy.
 */
const char *sched_policy_to_string(sched_policy_t policy);

#endif /* SCHEDULER_H */
