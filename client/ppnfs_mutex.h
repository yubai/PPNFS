/*
 *
 *  Created on: 21 oct. 2009
 *      Author: francois
 *
 */

#ifndef _PPNFSMUTEX_H_
#define _PPNFSMUTEX_H_

/**
 * PPNFS MUTEX type
 */
struct ppnfs_mutex_t
{
    pthread_mutex_t mutex;
    pthread_t owner;
    const char* context;
};

/**
 * ppnfs mutex interface, based on pthread_mutex
 */
void ppnfs_mutex_init ( struct ppnfs_mutex_t* mutex );
void ppnfs_mutex_destroy ( struct ppnfs_mutex_t* mutex, const char* name );

void ppnfs_mutex_lock ( struct ppnfs_mutex_t* mutex, const char* name, const char* context );
void ppnfs_mutex_unlock ( struct ppnfs_mutex_t* mutex, const char* name, const char* context );
void ppnfs_mutex_check_locked ( struct ppnfs_mutex_t* mutex, const char* name, const char* context );
void ppnfs_mutex_check_unlocked ( struct ppnfs_mutex_t* mutex, const char* name, const char* context );

extern struct ppnfs_mutex_t ppnfs_metadata_mutex;

#define __CONTEXT __FUNCTION__
#define ppnfs_metadata_lock() \
    ppnfs_mutex_lock ( &ppnfs_metadata_mutex, "metadata", __CONTEXT );
#define ppnfs_metadata_unlock() \
    ppnfs_mutex_unlock ( &ppnfs_metadata_mutex, "metadata", __CONTEXT );
#define ppnfs_metadata_check_locked() \
    ppnfs_mutex_check_locked ( &ppnfs_metadata_mutex, "metadata", __CONTEXT );
#define ppnfs_metadata_check_unlocked() \
    ppnfs_mutex_check_unlocked ( &ppnfs_metadata_mutex, "metadata", __CONTEXT );

#endif /* PPNFSMUTEX_H_ */
