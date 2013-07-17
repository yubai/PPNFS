/*
 *
 *  Created on: 21 oct. 2009
 *      Author: francois
 *
 */

#ifndef _PPNFSLOG_H_
#define _PPNFSLOG_H_

#define __PPNFS_USES_STDLOG

#ifdef __PPNFS_USES_STDLOG
#include <stdio.h>
#define PPNFS_OUTPUT_FD stderr
#endif

#define __Log(__prefix, __fmt, ...)                                     \
    do {                                                                \
        fprintf(PPNFS_OUTPUT_FD, __prefix "|" __FILE__ ":%d:%s|" __fmt, \
                __LINE__,  __FUNCTION__,                                \
                ##__VA_ARGS__ );                                        \
        fflush(PPNFS_OUTPUT_FD);                                        \
       } while(0)

/* #define Log(...) __Log("LOG", __VA_ARGS__) */
#ifdef DEBUG
#define Log(...) __Log("LOG", __VA_ARGS__)
#else
#define Log(...)
#endif

#define Info(...) __Log("INF", __VA_ARGS__)
#define Warn(...) __Log("WRN", __VA_ARGS__)
#define Err(...)  __Log("ERR", __VA_ARGS__)
#define Bug(...) do { __Log("BUG",__VA_ARGS__); ((char*)NULL)[0] = 0; } while(0)

#endif
