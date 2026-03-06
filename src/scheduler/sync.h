/**
 * @file sync.h
 * @brief Cooperative Mutex for inter-task synchronization
 *
 * Provides a simple mutex primitive for a cooperative (non-preemptive)
 * scheduler.  When a task attempts to lock a mutex held by another task,
 * it is moved to the WAITING state until the mutex is released.
 */

#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>
#include <stdint.h>

/** Invalid task ID sentinel */
#define MUTEX_NO_OWNER UINT32_MAX

/**
 * @brief Mutex structure.
 */
typedef struct {
  bool locked;       /**< true when mutex is held            */
  uint32_t owner_id; /**< Task ID of current owner           */
  char name[32];     /**< Human-readable mutex name          */
} mutex_t;

/**
 * @brief Initialise a mutex.
 * @param mtx   Pointer to mutex
 * @param name  Human-readable name (for logging)
 */
void mutex_init(mutex_t *mtx, const char *name);

/**
 * @brief Attempt to acquire the mutex.
 * @param mtx      Pointer to mutex
 * @param task_id  ID of the requesting task
 * @return true if lock acquired, false if already held by another task
 */
bool mutex_lock(mutex_t *mtx, uint32_t task_id);

/**
 * @brief Release the mutex.
 * @param mtx      Pointer to mutex
 * @param task_id  ID of the releasing task
 * @return true if released successfully, false if caller is not the owner
 */
bool mutex_unlock(mutex_t *mtx, uint32_t task_id);

/**
 * @brief Check whether the mutex is currently held.
 */
bool mutex_is_locked(const mutex_t *mtx);

#endif /* SYNC_H */
