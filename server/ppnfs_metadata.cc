/*
*
*  Created on: 21 oct. 2009
*      Author: francois
*
*/

#include "ppnfs_server.h"

#define __CLEANUP_HASH(__mdata) \
    do { __mdata->up = __mdata->left = __mdata->right = 0; } while (0)

struct ppnfs_metadata_t* ppnfs_metadata = NULL;

static const ppnfs_metadata_id ppnfs_metadata_id_root = 1;

int ppnfs_metadata_fd = -1;
size_t ppnfs_metadata_size = 0;

/**
 * **************************************** VERY LOW LEVEL *****************************************
 * format, open and close the metafile
 */
static void
ppnfs_metadata_format()
{
    struct ppnfs_metadata_t mdata;
    struct ppnfs_metadata_head_t mhead;

    memset(&mhead, 0, sizeof(mhead));
    mhead.block_bit_nr = BLOCK_BIT_NR;
    mhead.alloced_nb = 2;
    mhead.first_free = 0;
    strcpy(mhead.magic, "ppnfs.metafile.0");

    pwrite(ppnfs_metadata_fd, &mhead, sizeof(struct ppnfs_metadata_head_t), 0);

    memset(&mdata, 0, sizeof(struct ppnfs_metadata_t));

    mdata.id = 1;
    strcpy(mdata.d_name, "/");
    mdata.hash = doHash("/");

    if (stat(ppnfs_target, &(mdata.st))) {
        Err ( "Could not stat target : '%s'\n", ppnfs_target );
        exit(-1);
    }
    mdata.st.st_ino = 0;

    pwrite(ppnfs_metadata_fd, &mdata, sizeof(struct ppnfs_metadata_t),
           sizeof(struct ppnfs_metadata_t));
}

void
ppnfs_metadata_init()
{
    struct stat st;

    // if (stat(ppnfs_metafile, &st) == 0 && st.st_size) {
    //     ppnfs_metadata_fd = open(ppnfs_metafile, O_RDWR);
    //     if ( ppnfs_metadata_fd == -1 ) {
    //         Err ( "Could not open '%s' : err=%d:%s\n",
    //               ppnfs_metafile, errno, strerror(errno));
    //     }
    //     ppnfs_metadata_size = st.st_size;
    // }
    // else {
    ppnfs_metadata_fd =
            open(ppnfs_metafile,
                 O_CREAT | O_TRUNC | O_RDWR,
                 (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH));
     // }

    if (ppnfs_metadata_fd == -1) {
        Err ( "Could not open metafile '%s'\n", ppnfs_metafile );
        exit(-1);
    }

    if (ppnfs_metadata_size == 0) {

        ppnfs_metadata_format();

        if (fstat(ppnfs_metadata_fd, &st)) {
            Err ( "Could not fstat() formatted file.\n" );
            exit(-1);
        }
        ppnfs_metadata_size = st.st_size;
    }
    else {
        Log ( "Openned file, size=%llu\n",
            (unsigned long long) ppnfs_metadata_size );
    }

    ppnfs_metadata =
        (struct ppnfs_metadata_t*)mmap(NULL, ppnfs_metadata_size,
                                       PROT_READ | PROT_WRITE, MAP_SHARED,
                                       ppnfs_metadata_fd, 0);

    if (ppnfs_metadata == NULL || ppnfs_metadata == MAP_FAILED) {
        Err ( "Could not open metadata !\n" );
        exit(-1);
    }

    struct ppnfs_metadata_head_t* head = (struct ppnfs_metadata_head_t*)ppnfs_metadata;

    Log ( "Open metadata head structure: head=%p, alloced_nb=%llu, first_free=%llu\n",
          head, head->alloced_nb, head->first_free );

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
    if (id * sizeof(struct ppnfs_metadata_t) >= ppnfs_metadata_size) {
        Bug ( "Out of bounds : id=%llu\n", id );
    }

    return &(ppnfs_metadata[id]);
}

struct ppnfs_metadata_t*
ppnfs_metadata_get_root()
{
    return ppnfs_metadata_get(ppnfs_metadata_id_root);
}

static void
ppnfs_metadata_extend()
{
    struct ppnfs_metadata_t* metadata_old = ppnfs_metadata;

    struct ppnfs_metadata_head_t* head =
        (struct ppnfs_metadata_head_t*) ppnfs_metadata;

    struct ppnfs_metadata_t* newmd;

    ppnfs_metadata_id alloced_nb = head->alloced_nb, first_free = head->alloced_nb;

    ppnfs_metadata_id current;

    Log ( "Could not allocate, now extend file.\n" );

    munmap(ppnfs_metadata, ppnfs_metadata_size);

    alloced_nb += 64;

    if (ftruncate(ppnfs_metadata_fd, alloced_nb
                  * sizeof(struct ppnfs_metadata_t))) {
        Bug ( "Could not ftruncate up to %llu records : err=%d:%s\n",
              alloced_nb, errno, strerror(errno) );
    }

    ppnfs_metadata_size = alloced_nb * sizeof(struct ppnfs_metadata_t);

    ppnfs_metadata =
        (struct ppnfs_metadata_t*)mmap(ppnfs_metadata, ppnfs_metadata_size,
                                       PROT_READ | PROT_WRITE, MAP_SHARED,
                                       ppnfs_metadata_fd, 0);

    if (ppnfs_metadata == NULL || ppnfs_metadata == MAP_FAILED) {
        Err ( "Could not open metadata !\n" );
        exit(-1);
    }

    Log ( "RE-Openned metafile '%s'\n", ppnfs_metafile );

    head = (struct ppnfs_metadata_head_t*) ppnfs_metadata;
    head->first_free = first_free;
    head->alloced_nb = alloced_nb;

    for (current = first_free; current < alloced_nb; current++) {
        newmd = ppnfs_metadata_get(current);
        memset(newmd, 0, sizeof(struct ppnfs_metadata_t));
        newmd->id = current;
        newmd->next = current + 1 < alloced_nb ? current + 1 : 0;
    }

    if (metadata_old != ppnfs_metadata) {
        Log ( "metadata : remapped ppnfs_metadata from %p to %p\n",
              metadata_old, ppnfs_metadata );
    }
}

ppnfs_metadata_id
ppnfs_metadata_allocate()
{
    struct ppnfs_metadata_head_t* head =
        (struct ppnfs_metadata_head_t*) ppnfs_metadata;

    struct ppnfs_metadata_t* next;

    if (!head->first_free) {
        ppnfs_metadata_extend();
        head = (struct ppnfs_metadata_head_t*) ppnfs_metadata;
    }
    next = ppnfs_metadata_get(head->first_free);
    head->first_free = next->next;

    if (next->id == 0) {
        Bug ( "Invalid next !\n" );
    }

    Log ( "Allocate : provided next=%llu, next->next=%llu\n", next->id, next->next );
    return next->id;
}

static bool
ppnfs_metadata_equals(struct ppnfs_metadata_t* mdata, const char* path, int path_size)
{
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

/**
 * **************************************** HASH FUNCTIONS *****************************************
 * get, insert, remove, find
 */
void
ppnfs_metadata_insert_hash(struct ppnfs_metadata_t* newmd)
{
    __CLEANUP_HASH ( newmd );
    struct ppnfs_metadata_t* current = ppnfs_metadata_get_root();

    while (1) {
        if (newmd->hash < current->hash) {
            if (current->left) {
                current = ppnfs_metadata_get(current->left);
                continue;
            }
            current->left = newmd->id;
            newmd->up = current->id;

            Log ( "HASH Insert (%llu)->left=%llu\n", current->id, newmd->id );
            return;
        }
        else if (current->hash < newmd->hash) {
            if (current->right) {
                current = ppnfs_metadata_get(current->right);
                continue;
            }
            current->right = newmd->id;
            newmd->up = current->id;

            Log ( "HASH Insert (%llu)->right=%llu\n", current->id, newmd->id );
            return;

        }
        // Here we know that current->hash == newmd->hash
        if (current->collision_next) {
            current = ppnfs_metadata_get(current->collision_next);
            continue;
        }
        Err ( "Added collider : cur=%llu '%s', new=%llu '%s', hash='%llx'\n",
              current->id, current->d_name,
              newmd->id, newmd->d_name, current->hash );
        Bug ( "Bug because added collider (? ? ?)\n" );
        current->collision_next = newmd->id;
        newmd->collision_previous = current->id;
        return;
    }
}

void
ppnfs_metadata_remove_hash(struct ppnfs_metadata_t* mdata)
{
    struct ppnfs_metadata_t* up, *child, *left, *right, *cup, *cleft;
    int iamleft;

    if (mdata->collision_previous) {
        Log ( "Removing a collider, prev=%llu\n", mdata->collision_previous );
        left = ppnfs_metadata_get(mdata->collision_previous);
        left->collision_next = mdata->collision_next;
        if (mdata->collision_next) {
            right = ppnfs_metadata_get(mdata->collision_next);
            right->collision_previous = mdata->collision_previous;
        }
        return;
    }

    if (mdata->up == 0) {
        Bug ( "Attempt to remove the '/' hash !\n" );
    }

    up = ppnfs_metadata_get(mdata->up);
    iamleft = (up->left == mdata->id);

    if (!mdata->left && !mdata->right) {
        if (iamleft)
            up->left = 0;
        else
            up->right = 0;
        __CLEANUP_HASH(mdata);
        return;
    }

    if (!mdata->left || !mdata->right) {
        child = ppnfs_metadata_get(mdata->left ? mdata->left : mdata->right);
        child->up = up->id;
        if (iamleft)
            up->left = child->id;
        else
            up->right = child->id;
        __CLEANUP_HASH(mdata);
        return;
    }

    left = ppnfs_metadata_get(mdata->left);
    right = ppnfs_metadata_get(mdata->right);

    if (!left->right) {
        // Most simple case, where our left child has no right
        if (iamleft)
            up->left = left->id;
        else
            up->right = left->id;
        left->up = up->id;

        left->right = mdata->right;
        right->up = left->id;
        __CLEANUP_HASH(mdata);
        return;
    }

    if (!right->left) {
        // Second simple case, where our right child has no left
        if (iamleft)
            up->left = right->id;
        else
            up->right = right->id;
        right->up = up->id;

        right->left = mdata->left;
        left->up = right->id;
        __CLEANUP_HASH(mdata);
        return;

    }

    for (child = left; child->right; child = ppnfs_metadata_get(child->right))
        ;
    // We now that child has no right
    if (child->right) {
        Bug ( "Child had a right !\n" );
    }

    // child now is the direct predecessor
    cup = ppnfs_metadata_get(child->up);
    if (cup->hash == child->hash) {
        Bug ( "Hash collision detected in hash remove !\n" );
    }

    if (cup->right != child->id) {
        Bug ( "Invalid !\n" );
    }

    if (child->left) {
        cleft = ppnfs_metadata_get(child->left);
        cleft->up = cup->id;
    }
    cup->right = child->left;

    // Link my up's to this new child
    if (iamleft)
        up->left = child->id;
    else
        up->right = child->id;
    child->up = up->id;

    // Link the child's right to the right of the mdata to remove
    child->right = right->id;
    right->up = child->id;

    // Link the child's left to the left of the mdata to remove
    child->left = left->id;
    left->up = child->id;

    __CLEANUP_HASH(mdata);
}

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

static void
ppnfs_metadata_build_hash(struct ppnfs_metadata_t* father,
                          struct ppnfs_metadata_t* child)
{
    child->hash = father->father ? continueHash(father->hash, "/") : father->hash;
    child->hash = continueHash(child->hash, child->d_name);
}

/**
 * **************************************** TREE FUNCTIONS *****************************************
 * add, fetch, interpolate (to be done)
 */
void
ppnfs_metadata_add_child(struct ppnfs_metadata_t* father,
                         struct ppnfs_metadata_t* child)
{
    child->father = father->id;

    if (father->child == 0 || father->child == ppnfs_metadata_id_EMPTY) {
        child->next = 0;
    }
    else {
        child->next = father->child;
    }
    father->child = child->id;

    ppnfs_metadata_build_hash(father, child);
    ppnfs_metadata_insert_hash(child);
}

int
ppnfs_metadata_recurse_open(struct ppnfs_metadata_t* father)
{
    int fd, fd2;

    if (!father->father) {
        return open(ppnfs_target, O_RDONLY);
    }
    fd = ppnfs_metadata_recurse_open(ppnfs_metadata_get(father->father));
    if (fd == -1)
        return fd;
    fd2 = openat(fd, father->d_name, O_RDONLY);
    if (fd2 == -1) {
        Err ( "Could not open '%s', err=%d:%s\n", father->d_name, errno, strerror(errno) );
    }

    Log ( "fd=%d, fd2=%d, d_name=%s\n", fd, fd2, father->d_name );
    close(fd);
    return fd2;
}

struct ppnfs_metadata_t*
ppnfs_metadata_get_child(struct ppnfs_metadata_t* father)
{
    int fd;
    DIR* dp;
    struct dirent *de;
    ppnfs_metadata_id fatherid = father->id, newid;

    struct ppnfs_metadata_t* newmeta = NULL;

    if (father->child == ppnfs_metadata_id_EMPTY) {
        Log ( "Explicit EMPTY!\n" );
        return NULL;
    } else if (father->child) {
        Log ( "Explicit child.\n" );
        return ppnfs_metadata_get(father->child);
    }
    Log ( "We must lookup child for father=%s (%p:%llu)\n", father->d_name, father, father->id );

    // if (!S_ISDIR( father->st.st_mode )) {
    //     Err ( "Record %p:%llu not a directory !\n", father, father->id );
    //     return NULL;
    // }

    fd = ppnfs_metadata_recurse_open(father);
    if (fd == -1) {
        Err ( "Could not recurse open !\n" );
        return NULL;
    }

    dp = fdopendir(fd);

    if (dp == NULL) {
        Err ( "Could not fdopendir() : err=%d:%s\n", errno, strerror(errno));
        close(fd);
        return NULL;
    }

    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;
        //
        // Be really carefull, as ppnfs_metadata_allocate blurs the pointers!!!
        //
        newid = ppnfs_metadata_allocate();
        newmeta = ppnfs_metadata_get(newid);
        father = ppnfs_metadata_get(fatherid);

        strncpy(newmeta->d_name, de->d_name, NAME_MAX + 1);

        Log ( "de->d_name=%s\n", de->d_name );

        if (fstatat(fd, newmeta->d_name, &(newmeta->st), AT_SYMLINK_NOFOLLOW)) {
            Err ( "Could not fstatat(%d, %s, %p) : err=%d:%s\n",
                  fd, newmeta->d_name, &(newmeta->st), errno, strerror(errno) );
        }
        newmeta->st.st_ino = 0;
        ppnfs_metadata_add_child(father, newmeta);
    }
    closedir(dp);
    close(fd);

    if (!father->child) {
        father->child = ppnfs_metadata_id_EMPTY;
        return NULL;
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


    for (; child; child = ppnfs_metadata_get(child->next))
    {

        Log ( "walk_down, at child='%s'\n", child->d_name );

        if (child->hash == hash && strncmp(child->d_name, rpath, rpath_size) == 0)
        {
            Log ( "==> Found it !\n" );
            if (rpath[rpath_size])
            {
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
 * **************************************** DUMP FUNCTIONS *****************************************
 * Dump the contents of the meta file
 */

void
ppnfs_metadata_dump(struct ppnfs_metadata_t* meta)
{
    printf( "Metadata:id=%llu, name=%s, child id=%llu", meta->id, meta->d_name, meta->child );
}

static void ppnfs_metadata_hashtree_dump(queue<struct ppnfs_metadata_t*> metas)
{
    if (!metas.empty()) {
        size_t size = metas.size();

        struct ppnfs_metadata_t* mfather = NULL;

        for (int i = 0; i < size; ++i) {
            mfather = metas.front();
            ppnfs_metadata_dump(mfather);

            if (S_ISDIR( mfather->st.st_mode )) {
                struct ppnfs_metadata_t* mchild;
                for (mchild = ppnfs_metadata_get_child(mfather); mchild;
                     mchild = ppnfs_metadata_get(mchild->next)) {
                    metas.push(mchild);
                }
            }
            printf ("\t");
            metas.pop();
        }

        printf ("\n");
        ppnfs_metadata_hashtree_dump(metas);
    }
}

void
ppnfs_metadata_hashtree_dump()
{
    Log ( "--Metadata Hashtree Dump--\n" );
    queue<struct ppnfs_metadata_t*> metas;
    struct ppnfs_metadata_t *rootmeta = NULL;
    rootmeta = ppnfs_metadata_get_root();

    Log ( "metadata : %p\n", ppnfs_metadata );

    if (!rootmeta) {
        Bug ( "Could not get the root metadata!\n" );
    }

    metas.push(rootmeta);
    ppnfs_metadata_hashtree_dump(metas);
}
