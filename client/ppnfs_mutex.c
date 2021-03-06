/*
 *
 *  Created on: 21 oct. 2009
 *      Author: francois
 *
 */

#include "ppnfs_client.h"

#define PPNFS_MUTEX_INITIALIZER  \
    { PTHREAD_MUTEX_INITIALIZER, 0, NULL }

struct ppnfs_mutex_t ppnfs_metadata_mutex = PPNFS_MUTEX_INITIALIZER;

void
ppnfs_mutex_init(struct ppnfs_mutex_t* mutex)
{
    int res;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if ((res = pthread_mutex_init(&(mutex->mutex), &attr)) != 0)
    {
        Bug ( "Could not init mutex : err=%d:%s\n", res, strerror(res) );
    }
}

void
ppnfs_mutex_destroy(struct ppnfs_mutex_t* mutex, const char* name)
{
    int res;
    if ((res = pthread_mutex_destroy(&(mutex->mutex))) != 0)
    {
        Err ( "Could not destroy mutex '%s' : err=%d:%s, locked at %s by %lx\n",
              name, res, strerror(res), mutex->context, (unsigned long) mutex->owner );
    }
}

void
ppnfs_mutex_lock(struct ppnfs_mutex_t* mutex, const char* name,
                 const char* context)
{
    int res;
    pthread_t me = pthread_self();
    res = pthread_mutex_lock(&(mutex->mutex));
    if (res == 0)
    {
        mutex->owner = me;
        mutex->context = context;
        return;
    }
    Bug ( "Could not lock mutex '%s' at %s by %lx : err=%d:%s, locked at %s by %lx\n",
          name, context, (unsigned long) me, res, strerror(res), mutex->context,
          (unsigned long) mutex->owner );
}

void
ppnfs_mutex_check_unlocked(struct ppnfs_mutex_t* mutex, const char* name,
                           const char* context)
{
    pthread_t me = pthread_self();
    if (mutex->owner == me)
    {
        Bug ( "Mutex '%s' locked by myself %lx at %s : locked by %s at %lx\n",
              name, (unsigned long) me, context, mutex->context, (unsigned long) mutex->owner );
    }
}

void
ppnfs_mutex_check_locked(struct ppnfs_mutex_t* mutex, const char* name,
                         const char* context)
{
    pthread_t me = pthread_self();
    if (mutex->owner != me)
    {
        Bug ( "Mutex '%s' not locked by myself %lx at %s : locked by %s at %lx\n",
              name, (unsigned long) me, context, mutex->context, (unsigned long) mutex->owner );
    }
}

void
ppnfs_mutex_unlock(struct ppnfs_mutex_t* mutex, const char* name,
                   const char* context)
{
    int res;
    pthread_t me = pthread_self();

    if (mutex->owner != me)
    {
        Bug ( "Mutex '%s' was not held by myself ! locked by %lx at %s, unlocked by %lx at %s\n",
              name, (unsigned long)mutex->owner, mutex->context,
              (unsigned long)me, context );
    }

    mutex->owner = 0;
    mutex->context = NULL;
    res = pthread_mutex_unlock(&(mutex->mutex));
    if (res == 0)
    {
        return;
    }
    Bug ( "Could not unlock mutex '%s' by %lx at %s : err=%d:%s, locked by %lx at %s\n",
          name, (unsigned long)me, context, res, strerror(res), (unsigned long)mutex->owner,
          mutex->context );

}
