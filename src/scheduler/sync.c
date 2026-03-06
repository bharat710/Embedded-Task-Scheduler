/**
 * @file sync.c
 * @brief Cooperative Mutex Implementation
 *
 * Provides a simple lock/unlock mechanism for cooperative scheduling.
 * If a task cannot acquire the lock, the caller is expected to move
 * that task to WAITING state externally.
 */

#include "sync.h"
#include <stdio.h>
#include <string.h>

#define CLR_RESET "\033[0m"
#define CLR_YELLOW "\033[1;33m"
#define CLR_GREEN "\033[1;32m"

void mutex_init(mutex_t *mtx, const char *name) {
  if (!mtx)
    return;

  mtx->locked = false;
  mtx->owner_id = MUTEX_NO_OWNER;
  strncpy(mtx->name, name ? name : "mutex", sizeof(mtx->name) - 1);
  mtx->name[sizeof(mtx->name) - 1] = '\0';
}

bool mutex_lock(mutex_t *mtx, uint32_t task_id) {
  if (!mtx)
    return false;

  if (!mtx->locked) {
    mtx->locked = true;
    mtx->owner_id = task_id;
    printf("           %s🔒 [Task %u] Acquired mutex '%s'%s\n", CLR_GREEN,
           task_id, mtx->name, CLR_RESET);
    return true;
  }

  if (mtx->owner_id == task_id) {
    /* Already held by this task – re-entrant success */
    return true;
  }

  printf("           %s🔒 [Task %u] Blocked on mutex '%s' (held by %u)%s\n",
         CLR_YELLOW, task_id, mtx->name, mtx->owner_id, CLR_RESET);
  return false;
}

bool mutex_unlock(mutex_t *mtx, uint32_t task_id) {
  if (!mtx)
    return false;

  if (mtx->owner_id != task_id) {
    fprintf(stderr, "  [WARN] Task %u tried to unlock mutex '%s' owned by %u\n",
            task_id, mtx->name, mtx->owner_id);
    return false;
  }

  mtx->locked = false;
  mtx->owner_id = MUTEX_NO_OWNER;
  printf("           %s🔓 [Task %u] Released mutex '%s'%s\n", CLR_GREEN,
         task_id, mtx->name, CLR_RESET);
  return true;
}

bool mutex_is_locked(const mutex_t *mtx) { return mtx ? mtx->locked : false; }
