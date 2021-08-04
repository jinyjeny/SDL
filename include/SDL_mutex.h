/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL_mutex_h_
#define SDL_mutex_h_

/**
 *  \file SDL_mutex.h
 *
 *  Functions to provide thread synchronization primitives.
 */

#include "SDL_stdinc.h"
#include "SDL_error.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Synchronization functions which can time out return this value
 *  if they time out.
 */
#define SDL_MUTEX_TIMEDOUT  1

/**
 *  This is the timeout value which corresponds to never time out.
 */
#define SDL_MUTEX_MAXWAIT   (~(Uint32)0)


/**
 *  \name Mutex functions
 */
/* @{ */

/* The SDL mutex structure, defined in SDL_sysmutex.c */
struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;

/**
 *  Create a mutex, initialized unlocked.
 */
extern DECLSPEC SDL_mutex *SDLCALL SDL_CreateMutex(void);

/**
 *  Lock the mutex.
 *
 * It is legal for the owning thread to lock an already-locked mutex. It must
 * unlock it the same number of times before it is actually made available for
 * other threads in the system (this is known as a "recursive mutex").
 *
 * \param mutex the mutex to lock
 * \return 0, or -1 on error.
 */
#define SDL_mutexP(m)   SDL_LockMutex(m)
extern DECLSPEC int SDLCALL SDL_LockMutex(SDL_mutex * mutex);

/**
 * Try to lock a mutex without blocking.
 *
 * This works just like SDL_LockMutex(), but if the mutex is not available,
 * this function returns `SDL_MUTEX_TIMEOUT` immediately.
 *
 * This technique is useful if you need exclusive access to a resource but
 * don't want to wait for it, and will return to it to try again later.
 *
 * \param mutex the mutex to try to lock
 * \returns 0, `SDL_MUTEX_TIMEDOUT`, or -1 on error; call SDL_GetError() for
 *          more information.
 *
 *  \return 0, SDL_MUTEX_TIMEDOUT, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_TryLockMutex(SDL_mutex * mutex);

/**
 *  Unlock the mutex.
 *
 * It is legal for the owning thread to lock an already-locked mutex. It must
 * unlock it the same number of times before it is actually made available for
 * other threads in the system (this is known as a "recursive mutex").
 *
 *  \warning It is an error to unlock a mutex that has not been locked by
 *           the current thread, and doing so results in undefined behavior.
 */
#define SDL_mutexV(m)   SDL_UnlockMutex(m)
extern DECLSPEC int SDLCALL SDL_UnlockMutex(SDL_mutex * mutex);

/**
 *  Destroy a mutex.
 */
extern DECLSPEC void SDLCALL SDL_DestroyMutex(SDL_mutex * mutex);

/* @} *//* Mutex functions */


/**
 *  \name Semaphore functions
 */
/* @{ */

/* The SDL semaphore structure, defined in SDL_syssem.c */
struct SDL_semaphore;
typedef struct SDL_semaphore SDL_sem;

/**
 *  Create a semaphore, initialized with value, returns NULL on failure.
 */
extern DECLSPEC SDL_sem *SDLCALL SDL_CreateSemaphore(Uint32 initial_value);

/**
 *  Destroy a semaphore.
 */
extern DECLSPEC void SDLCALL SDL_DestroySemaphore(SDL_sem * sem);

/**
 *  This function suspends the calling thread until the semaphore pointed
 *  to by \c sem has a positive count. It then atomically decreases the
 *  semaphore count.
 */
extern DECLSPEC int SDLCALL SDL_SemWait(SDL_sem * sem);

/**
 *  Non-blocking variant of SDL_SemWait().
 *
 *  \return 0 if the wait succeeds, ::SDL_MUTEX_TIMEDOUT if the wait would
 *          block, and -1 on error.
 */
extern DECLSPEC int SDLCALL SDL_SemTryWait(SDL_sem * sem);

/**
 * Wait until a semaphore has a positive value and then decrements it.
 *
 * This function suspends the calling thread until either the semaphore
 * pointed to by `sem` has a positive value, the call is interrupted by a
 * signal or error, or the specified time has elapsed. If the call is
 * successful it will atomically decrement the semaphore value.
 *
 *  \return 0 if the wait succeeds, ::SDL_MUTEX_TIMEDOUT if the wait does not
 *          succeed in the allotted time, and -1 on error.
 *
 *  \warning On some platforms this function is implemented by looping with a
 *           delay of 1 ms, and so should be avoided if possible.
 */
extern DECLSPEC int SDLCALL SDL_SemWaitTimeout(SDL_sem * sem, Uint32 ms);

/**
 *  Atomically increases the semaphore's count (not blocking).
 *
 *  \return 0, or -1 on error.
 */
extern DECLSPEC int SDLCALL SDL_SemPost(SDL_sem * sem);

/**
 *  Returns the current count of the semaphore.
 */
extern DECLSPEC Uint32 SDLCALL SDL_SemValue(SDL_sem * sem);

/* @} *//* Semaphore functions */


/**
 *  \name Condition variable functions
 */
/* @{ */

/* The SDL condition variable structure, defined in SDL_syscond.c */
struct SDL_cond;
typedef struct SDL_cond SDL_cond;

/**
 *  Create a condition variable.
 *
 *  Typical use of condition variables:
 *
 *  Thread A:
 *    SDL_LockMutex(lock);
 *    while ( ! condition ) {
 *        SDL_CondWait(cond, lock);
 *    }
 *    SDL_UnlockMutex(lock);
 *
 *  Thread B:
 *    SDL_LockMutex(lock);
 *    ...
 *    condition = true;
 *    ...
 *    SDL_CondSignal(cond);
 *    SDL_UnlockMutex(lock);
 *
 *  There is some discussion whether to signal the condition variable
 *  with the mutex locked or not.  There is some potential performance
 *  benefit to unlocking first on some platforms, but there are some
 *  potential race conditions depending on how your code is structured.
 *
 *  In general it's safer to signal the condition variable while the
 *  mutex is locked.
 */
extern DECLSPEC SDL_cond *SDLCALL SDL_CreateCond(void);

/**
 *  Destroy a condition variable.
 */
extern DECLSPEC void SDLCALL SDL_DestroyCond(SDL_cond * cond);

/**
 *  Restart one of the threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
extern DECLSPEC int SDLCALL SDL_CondSignal(SDL_cond * cond);

/**
 *  Restart all threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
extern DECLSPEC int SDLCALL SDL_CondBroadcast(SDL_cond * cond);

/**
 *  Wait on the condition variable, unlocking the provided mutex.
 *
 * This function unlocks the specified `mutex` and waits for another thread to
 * call SDL_CondSignal() or SDL_CondBroadcast() on the condition variable
 * `cond`. Once the condition variable is signaled, the mutex is re-locked and
 * the function returns.
 *
 *  The mutex is re-locked once the condition variable is signaled.
 *
 *  \return 0 when it is signaled, or -1 on error.
 */
extern DECLSPEC int SDLCALL SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex);

/**
 * Wait until a condition variable is signaled or a certain time has passed.
 *
 * This function unlocks the specified `mutex` and waits for another thread to
 * call SDL_CondSignal() or SDL_CondBroadcast() on the condition variable
 * `cond`, or for the specified time to elapse. Once the condition variable is
 * signaled or the time elapsed, the mutex is re-locked and the function
 * returns.
 *
 * The mutex must be locked before calling this function.
 *
 * \param cond the condition variable to wait on
 * \param mutex the mutex used to coordinate thread access
 * \param ms the maximum time to wait, in milliseconds, or `SDL_MUTEX_MAXWAIT`
 *           to wait indefinitely
 * \returns 0 if the condition variable is signaled, `SDL_MUTEX_TIMEDOUT` if
 *          the condition is not signaled in the allotted time, or a negative
 *          error code on failure; call SDL_GetError() for more information.
 *
 *  \warning On some platforms this function is implemented by looping with a
 *           delay of 1 ms, and so should be avoided if possible.
 */
extern DECLSPEC int SDLCALL SDL_CondWaitTimeout(SDL_cond * cond,
                                                SDL_mutex * mutex, Uint32 ms);

/* @} *//* Condition variable functions */


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_mutex_h_ */

/* vi: set ts=4 sw=4 expandtab: */
