/*
 *
 *  Created on: 21 oct. 2009
 *      Author: francois
 *  Modified on: 2012
 *      Author: Bai YU
 *
 */

#ifndef _PPNFSMETADATA_H_
#define _PPNFSMETADATA_H_

static const ppnfs_metadata_id ppnfs_metadata_id_EMPTY = ~((ppnfs_metadata_id)0);

struct ppnfs_metadata_head_t
{
    char magic[64];
    unsigned int bit_per_blk;
    ppnfs_metadata_id alloced_nb;
    ppnfs_metadata_id first_free;
};

struct ppnfs_metadata_t
{
    hash_t hash;
    char d_name[NAME_MAX+1];

    ppnfs_metadata_id id;     //< Myself
    ppnfs_metadata_id father; //< Pointer to my father dir
    ppnfs_metadata_id child;  //< First child (head of the tree)
    ppnfs_metadata_id next;   //< Next child

    ppnfs_metadata_id up;     //< Hashtree father
    ppnfs_metadata_id left;   //< Hashtree left part, where all hashes are stricly inf
    ppnfs_metadata_id right;  //< Hashtree right part, where hashes are equal or sup (collision mgmt)

    ppnfs_metadata_id collision_next;     //< Collision : next in the linked-list of colliders
    ppnfs_metadata_id collision_previous; //< Collision : previous in the linked-list of colliders

    struct stat st;

    // GR-PIR variables
    unsigned int prime;
    unsigned int c;
    char pi[];  //< Each file is accociated with a @pi, here I use the flexible length of array.
};

/**
 * *********************** METADATA *******************************
 */

void ppnfs_client_metadata_init ();
void ppnfs_metadata_close ();

ppnfs_metadata_id ppnfs_metadata_allocate();

int ppnfs_metadata_recurse_open(struct ppnfs_metadata_t* father);

struct ppnfs_metadata_t* ppnfs_metadata_get ( ppnfs_metadata_id id );
struct ppnfs_metadata_t* ppnfs_metadata_get_root();
struct ppnfs_metadata_t* ppnfs_metadata_get_child ( struct ppnfs_metadata_t* father );
struct ppnfs_metadata_t* ppnfs_metadata_find_locked ( const char* path ); // Assert that ppnfs_metadata_lock IS locked
struct ppnfs_metadata_t* ppnfs_metadata_find ( const char* path ); // Locks ppnfs_metadata_lock, remains locked
void ppnfs_metadata_release ( struct ppnfs_metadata_t* mdata ); // Releases ppnfs_metadata_lock
int ppnfs_metadata_getattr(const char* path, struct stat* stbuf);

/* Allcate and return the full path of a given mdata */
char* ppnfs_metadata_get_path ( struct ppnfs_metadata_t* mdata );
void ppnfs_metadata_add_child(struct ppnfs_metadata_t* father, struct ppnfs_metadata_t* child);

/* Dump functions */
void ppnfs_metadata_dump(struct ppnfs_metadata_t* meta);
void ppnfs_metadata_hashtree_dump();

#endif
