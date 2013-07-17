/***
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *     2012 Bai Yu - zjuyubai@gmail.com
 */
#ifndef _PPNFSSERVER_H_
#define _PPNFSSERVER_H_

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <linux/limits.h>
#include <limits.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <queue>
#include <math.h>
#include <gmp.h>

#include <time.h>

using namespace std;

#define BLOCK_BIT_NR 128

#define DEBUG

// Hash values, used for hashing paths
typedef unsigned long long int hash_t;

// Metadata ID, used for representing a specific metadata
typedef unsigned long long ppnfs_metadata_id;

#include "ppnfs_init.h"
#include "ppnfs_log.h"
#include "ppnfs_util.h"
#include "ppnfs_metadata.h"
#include "ppnfs_mutex.h"
#include "ppnfs_grpir.h"
#include "Pohlig_hellman.h"
#include "ppnfs_net.h"

// Common variables
extern const char *ppnfs_target;
extern const char *ppnfs_metafile;
extern struct ppnfs_metadata_t* ppnfs_metadata;

#endif
