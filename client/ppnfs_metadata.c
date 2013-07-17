/*
 *
 *  Created on: 21 oct. 2009
 *      Author: francois
 *
 */

#include "ppnfs_client.h"

#define __CLEANUP_HASH(__mdata) \
    do { __mdata->up = __mdata->left = __mdata->right = 0; } while (0)

struct ppnfs_metadata_t* ppnfs_metadata = NULL;

static const ppnfs_metadata_id ppnfs_metadata_id_root = 1;

int ppnfs_metadata_fd = -1;
size_t ppnfs_metadata_size = 0;
size_t ppnfs_metadata_struct_size;

/**
 * **************************************** VERY LOW LEVEL *****************************************
 * format, open and close the metafile
 */

void
ppnfs_client_metadata_init()
{
    struct stat st;

    if (stat(ppnfs_metafile, &st) == 0 && st.st_size) {
        ppnfs_metadata_fd = open(ppnfs_metafile, O_RDWR);
        if ( ppnfs_metadata_fd == -1 ) {
            Err ( "Could not open '%s' : err=%d:%s\n", ppnfs_metafile, errno, strerror(errno));
        }
        ppnfs_metadata_size = st.st_size;
        Log ( "metadata file already exist!\n" );
    }
    else {
        Err ( "Could not open metafile '%s'\n", ppnfs_metafile );
    }

    Log ( "Openned file, size=%llu\n", (unsigned long long) ppnfs_metadata_size );

    ppnfs_metadata = (struct ppnfs_metadata_t*)mmap(NULL,
                                                    ppnfs_metadata_size,
                                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                                    ppnfs_metadata_fd, 0);

    if (ppnfs_metadata == NULL || ppnfs_metadata == MAP_FAILED) {
        Err ( "Could not open metadata !\n" );
        exit(-1);
    }

    struct ppnfs_metadata_t* current = ppnfs_metadata_get_root();

    Log ( "Root name=\"%s\"\n", current->d_name );
    Log ( "Openned metafile '%s'\n", ppnfs_metafile );
}

void
ppnfs_metadata_close()
{
    if (ppnfs_metadata) {
        if (munmap(ppnfs_metadata, ppnfs_metadata_size)) {
            Err ( "Could not munmap : err=%d:%s\n", errno, strerror(errno) );
            exit(-1);
        }
        ppnfs_metadata = NULL;
        ppnfs_metadata_size = 0;
    }
    if (ppnfs_metadata_fd != -1) {
        close(ppnfs_metadata_fd);
        ppnfs_metadata_fd = -1;
    }
}

/**
 * **************************************** LOW LEVEL *******************************************
 * fetch, extend and allocate
 */

struct ppnfs_metadata_t*
ppnfs_metadata_get(ppnfs_metadata_id id)
{
    if (!id)
        return NULL;
    if (id == ppnfs_metadata_id_EMPTY) {
        Bug ( "Attempt to fetch an EMPTY child !\n" );
    }
    if (id * ppnfs_metadata_struct_size >= ppnfs_metadata_size) {
        Bug ( "Out of bounds : id=%llu\n", id );
    }

    return (struct ppnfs_metadata_t*)((char*)ppnfs_metadata + id * ppnfs_metadata_struct_size);
}

static bool
ppnfs_metadata_equals(struct ppnfs_metadata_t* mdata, const char* path, int path_size)
{
//    return strcmp(mdata->d_name, path) ? false : true;
    // if (strcmp(mdata->d_name, path) == 0) {
    //     return true;
    // } else {
    //     return false;
    // }
    int path_start;

    while (1) {
        Log ( "Equals : mdata=%p:%s (father=%llu), path=%s, path_size=%d\n",
              mdata, mdata->d_name, mdata->father,
              path, path_size );

        if (path_size <= 1)
            return mdata->father == 0;

        path_start = path_size ? path_size - 1 : 0;
        while (path_start && path[path_start] != '/')
            path_start--;

        Log ( "=>path_start=%d\n", path_start );

        if (strncmp(mdata->d_name, &(path[path_start + 1]),
                    path_size - path_start - 1)) {
            Log ( "Colliding : mdata=%s, rpath=%s (up to %d)\n",
                  mdata->d_name, &(path[path_start+1]), path_size - path_start - 1 );
            return 0;
        }

        if (!mdata->father)
            return 0;

        mdata = ppnfs_metadata_get(mdata->father);
        path_size = path_start;
    }
    Bug ( "Shall not be here.\n" );
    return -1;

}

struct ppnfs_metadata_t*
ppnfs_metadata_get_root()
{
    return ppnfs_metadata_get(ppnfs_metadata_id_root);
}

/**
 * **************************************** HASH FUNCTIONS *****************************************
 * get, insert, remove, find
 */
static struct ppnfs_metadata_t*
ppnfs_metadata_find_hash(const char* path, hash_t hash, int path_size)
{
    struct ppnfs_metadata_t* current = ppnfs_metadata_get_root();
    Log ( "Finding hash for '%s' (up to %d chars) : hash=%llx \n", path, path_size, hash );

    while (current) {
        Log ( "At current=%p (%s), hash=%llx\n", current, current->d_name, current->hash );
        if (hash < current->hash) {
            current = ppnfs_metadata_get(current->left);
            continue;
        }
        if (current->hash < hash) {
            current = ppnfs_metadata_get(current->right);
            continue;
        }

        if (ppnfs_metadata_equals(current, path, path_size)) {
            return current;
        }
        Log ( "Got a collision with current=%llu:'%s' (hash=%llx), path=%s, hash=%llx!\n",
              current->id, current->d_name, current->hash, path, hash );
        current = ppnfs_metadata_get(current->collision_next);
    }
    return NULL;
}

struct ppnfs_metadata_t*
ppnfs_metadata_get_child(struct ppnfs_metadata_t* father)
{
    ppnfs_metadata_id fatherid = father->id;

    struct ppnfs_metadata_t* newmeta = NULL;

    if (father->child == ppnfs_metadata_id_EMPTY) {
        Log ( "Explicit EMPTY!\n" );
        return NULL;
    } else if (father->child) {
        Log ( "Explicit child.\n" );
        return ppnfs_metadata_get(father->child);
    }

    return ppnfs_metadata_get(father->child);
}

struct ppnfs_metadata_t*
ppnfs_metadata_walk_down(struct ppnfs_metadata_t* father,
                         const char* path, int path_size)
{
    const char* rpath = NULL;
    hash_t father_hash, hash;
    int rpath_size;
    ppnfs_metadata_id father_id;

walk_down_start:

    path_size++;
    rpath = &(path[path_size]);

    rpath_size = 0;
    while (rpath[rpath_size] != '\0' && rpath[rpath_size] != '/')
        rpath_size++;

    father_hash = father->hash;
    father_id = father->id;
    hash = 0;

    Log ( "May walk down : father=%p:%s, path=%s (path_size=%d), rpath=%s (rpath_size=%d)\n",
          father, father->d_name,
          path, path_size,
          rpath, rpath_size );

    struct ppnfs_metadata_t* child = ppnfs_metadata_get_child(father);
    father = NULL;

    if (!child)
        return NULL;

    Log ( "=> Now I will walk that list down.\n" );

    father = ppnfs_metadata_get(father_id);

    // father_hash = continueHash(father_hash, "/");
    // hash = continueHashPartial(father_hash, rpath, rpath_size);

    father_hash = father->father ? continueHash(father->hash, "/") : father->hash;
    hash = continueHashPartial(father_hash, rpath, rpath_size);

    for (; child; child = ppnfs_metadata_get(child->next)) {
        Log ( "walk_down, at child='%s'\n", child->d_name );

        if (child->hash == hash && strncmp(child->d_name, rpath, rpath_size) == 0) {
            Log ( "==> Found it !\n" );
            if (rpath[rpath_size]) {
                Log ( "rpath(rpath_size)=%c\n", rpath[rpath_size] );
                father = child;
                path_size += rpath_size + 1;
                goto walk_down_start;
            }
            return child;
        }
    }
    return NULL;
}

// typedef unsigned long long int hash_t;
struct ppnfs_metadata_t*
ppnfs_metadata_find_locked(const char* path)
{
    struct ppnfs_metadata_t* metadata;
    int path_size = strlen(path);
    hash_t hash = doHash(path);

    Log ( "Finding path '%s'\n", path );

    metadata = ppnfs_metadata_find_hash(path, hash, path_size);

    if (metadata) {
        Log ( "Found : id=%llu\n", metadata->id );
        return metadata;
    }

    Log ( "Could not get '%s'\n", path );

    while (1) {
        while (path_size && path[path_size] != '/') {
            path_size--;
        }
        if (path_size <= 1) {
            metadata = ppnfs_metadata_get_root();
            break;
        }
        hash = doHashPartial(path, path_size);

        metadata = ppnfs_metadata_find_hash(path, hash, path_size);

        if (metadata)
            break;
        path_size--;
    }

    Log ( "After semi-lookup : metadata=%p, path_size=%d\n", metadata, path_size );

    return ppnfs_metadata_walk_down(metadata, path, path_size);
}

struct ppnfs_metadata_t*
ppnfs_metadata_find(const char* path)
{
    struct ppnfs_metadata_t* metadata;
    ppnfs_metadata_lock ();
    metadata = ppnfs_metadata_find_locked(path);

    if (!metadata)
        ppnfs_metadata_unlock ();
    return metadata;
}

void
ppnfs_metadata_release(struct ppnfs_metadata_t* mdata)
{
    (void) mdata;
    ppnfs_metadata_unlock ();
}

/**
 * **************************************** HIGH-LEVEL FUNCTIONS ***********************************
 * remove children, flush entries, rename, mknod, rmdir, ...
 */
int
ppnfs_metadata_getattr(const char* path, struct stat* stbuf)
{
    struct ppnfs_metadata_t* mdata = ppnfs_metadata_find(path);

    if (!mdata) {
        Log ( "Could not find '%s'\n", path );
        return -ENOENT;
    }
    Log ( "Found '%llu' : '%s', (hash=%llx)\n", mdata->id, mdata->d_name, mdata->hash );

    memcpy(stbuf, &(mdata->st), sizeof(struct stat));

    ppnfs_metadata_release(mdata);

    if (S_ISDIR(stbuf->st_mode) && stbuf->st_size == 0) {
        stbuf->st_size = 1 << 12;
    }
    return 0;
}
