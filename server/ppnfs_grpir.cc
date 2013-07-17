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
#include "ppnfs_server.h"

// PPNFS block head structure
struct ppnfs_block_head_t {
    struct ppnfs_block_head_t *next;
    struct ppnfs_block_node_t *down;
};

// PPNFS file attribute structure
struct ppnfs_file_t {
    unsigned int prime;
    mpz_t pi;
    unsigned int c;
    struct ppnfs_metadata_t *mdata;
    struct ppnfs_file_t *next;
};

// PPNFS block node structure
struct ppnfs_block_node_t {
    mpz_t block;
    struct ppnfs_file_t *file;
    struct ppnfs_block_node_t *next;
    struct ppnfs_block_node_t *down;
};

// PPNFS partition structure
struct ppnfs_partition_t {
    mpz_t e;
    struct ppnfs_partition_t *next;
};

// PPNFS global infomation
struct ppnfs_globalinfo_t {
    unsigned int max_block_nr; // Maximum number of blocks for one file
}ppnfs_globalinfo;

struct ppnfs_block_head_t    *ppnfs_block_head;
struct ppnfs_block_node_t    *ppnfs_block_node;
struct ppnfs_file_t          *ppnfs_file_head;
struct ppnfs_partition_t     *ppnfs_partition_head;

static inline struct ppnfs_block_node_t*
ppnfs_block_node_alloc()
{
    struct ppnfs_block_node_t *current
        = (struct ppnfs_block_node_t *)malloc(sizeof(struct ppnfs_block_node_t));
    current->next = NULL;
    current->down = NULL;
    return current;
}

static inline struct ppnfs_file_t*
ppnfs_file_alloc()
{
    struct ppnfs_file_t *current
        = (struct ppnfs_file_t *)malloc(sizeof(struct ppnfs_file_t));
    current->next = NULL;
    return current;
}

static void
ppnfs_prime_gen(struct ppnfs_file_t *current)
{
    static int b;
    static mpz_t p[2];
    static int first = 1;

    if (first == 1) {
        mpz_init(p[0]);
        mpz_init(p[1]);
        mpz_set_ui(p[0], 2);
        b = 1;
        first = 0;
    }

    mpz_nextprime(p[b], p[1-b]);
    current->prime = mpz_get_ui(p[b]);
    b = 1-b;
}

static void
ppnfs_prime_dump()
{
    struct ppnfs_file_t *current = ppnfs_file_head;
    while (current != NULL) {
        printf("%u\t", current->pi);
        current = current->next;
    }
    printf("\n");
}

static void
ppnfs_pi_gen(struct ppnfs_file_t *current)
{
    static int first = 1;

    if (first == 1) {
        ppnfs_globalinfo.max_block_nr = 0;
        first = 0;
    }

    mpz_init(current->pi);
    current->c = ceil((double)(BLOCK_BIT_NR >> 1)/(log(current->prime)/log(2)) - 1e-6);
    mpz_ui_pow_ui(current->pi, current->prime, current->c);
}

static void
ppnfs_metafile_attr_init(struct ppnfs_metadata_t* mdata, struct ppnfs_file_t *pfile)
{
    gmp_sprintf(mdata->pi, "%Zx\n", pfile->pi);
    mdata->prime = pfile->prime;
    mdata->c = pfile->c;
}

static void
ppnfs_block_read(struct ppnfs_metadata_t *mfather,
                 struct ppnfs_metadata_t *mchild,
                 struct ppnfs_file_t     *file)
{

    static ppnfs_block_node_t **current;
    static int first = 1;

    if (first == 1) {
        current = &ppnfs_block_node;
        first = 0;
    }

    int fdfather, fdchild;

    fdfather = ppnfs_metadata_recurse_open ( mfather );
    fdchild = openat ( fdfather, mchild->d_name, O_RDONLY );

    if (fdchild == -1) {
        Err ( "Could not open '%s', err=%d:%s\n",
              mchild->d_name, errno, strerror(errno) );
        return;
    }

    FILE *fp = fdopen( fdchild, "r" );
    if (fp == NULL) {
        Err ( "Could not fdopendir() : err=%d:%s\n", errno, strerror(errno));
        fclose(fp);
        return;
    }

    // Update the value of the maximum number of the blocks in a file
    unsigned int blocks = ((mchild->st.st_size << 1) + (BLOCK_BIT_NR >> 3) - 1)
        / (BLOCK_BIT_NR >> 3);
    if (blocks > ppnfs_globalinfo.max_block_nr) {
        ppnfs_globalinfo.max_block_nr = blocks;
    }

    ppnfs_block_node_t **cursor = current;

    // Temp buffer to store the characters in the file
    int size = (BLOCK_BIT_NR >> 3) + 1;
    char *buf = (char*) malloc (size);

    while (blocks-- != 0) {

        *cursor = ppnfs_block_node_alloc();
        (*cursor)->file = file;

        memset(buf, 0, size);

        char *p = buf;
        for (int i = 0; i < (BLOCK_BIT_NR >> 4); ++i) {
            fscanf(fp, "%c", p++);
        }
        decompStr(buf);
        *(buf+size-1) = '\n';

        mpz_init((*cursor)->block);
        gmp_sscanf(buf, "%Zx\n", (*cursor)->block);

        cursor = &(*cursor)->next;
    }

    current = &(*current)->down;
}

static void
ppnfs_partition_block_dump(struct ppnfs_block_node_t *current)
{
    while(current != NULL) {
        gmp_printf("%Zx   ", current->block);
        current = current->down;
    }
    printf("\n");
}

// Finding a block structure according to the index
static struct ppnfs_block_node_t *
ppnfs_block_find(struct ppnfs_block_node_t *head, int index)
{
    struct ppnfs_block_node_t* current = head;
    while (current != NULL) {
        if (index-- == 0) return current;
        current = current->next;
    }
    return NULL;
}

static void
ppnfs_block_tree_build()
{
    struct ppnfs_block_head_t **current = &ppnfs_block_head;
    struct ppnfs_block_head_t *cursor;
    struct ppnfs_block_node_t *down;

    // Initialize the block headers
    for (unsigned int i = 0; i < ppnfs_globalinfo.max_block_nr; ++i) {
        (*current) = (struct ppnfs_block_head_t*) malloc (sizeof(ppnfs_block_head_t));
        (*current)->down = NULL;
        (*current)->next = NULL;
        current = &(*current)->next;
    }

    ppnfs_block_head->down = ppnfs_block_node;

    // Constructing the block tree
    current = &ppnfs_block_head;
    cursor = ppnfs_block_head;
    if (*current != NULL && (*current)->down != NULL) {
        int index = 0;
        while (cursor != NULL) {
            struct ppnfs_block_node_t *p = (*current)->down;
            if (p != NULL) {
                do {
                    down = ppnfs_block_find(p, index);
                    p = p->down;
                } while (down == NULL && p != NULL);
                cursor->down = down;
            }
            index++;
            cursor = cursor->next;
        }
    }

    struct ppnfs_block_node_t *head = ppnfs_block_node;
    struct ppnfs_block_node_t *currnode = head;

    while (head != NULL && head->down != NULL) {
        int index = 0;
        while (currnode != NULL) {
            struct ppnfs_block_node_t *p = head->down;
            if (p != NULL) {
                do {
                    down = ppnfs_block_find(p, index);
                    p = p->down;
                } while (down == NULL && p != NULL);
                currnode->down = down;
            }
            index++;
            currnode = currnode->next;
        }
        head = head->down;
        currnode = head;
    }
}

static inline void
ppnfs_print_line(int nr)
{
    while (nr--) {
        printf("--------");
    }
    printf ("\n");
}

static void
ppnfs_block_tree_dump()
{
    Log ("Dumping Tree Blocks\n");
    struct ppnfs_block_node_t *head = ppnfs_block_node;
    struct ppnfs_file_t *file = ppnfs_file_head;

    ppnfs_print_line(ppnfs_globalinfo.max_block_nr + 2);

    int index = 0;
    while (head != NULL && file != NULL) {
        printf ("FILE[%s]:\t", file->mdata->d_name);

        struct ppnfs_block_node_t *current = head;
        while (current != NULL) {
            gmp_printf("|%Zx|\t", current->block);
            current = current->next;
        }
        printf ("\n");
        head = head->down;
        file = file->next;
        index++;
    }

    printf ("\nPARTITION:\t");
    for (unsigned int i = 0; i < ppnfs_globalinfo.max_block_nr; ++i) {
        printf ("+-P%d+-\t", i);
    }
    printf ("\n");

    ppnfs_print_line(ppnfs_globalinfo.max_block_nr + 2);
}

// Chinese Remainder Theorem
static void
crt(struct ppnfs_partition_t *partition, struct ppnfs_block_head_t *blockhd)
{
    mpz_t g, ri, si, dv, ei;
    mpz_init(g);
    mpz_init(ri);
    mpz_init(si);
    mpz_init(ei);
    mpz_init(dv);
    unsigned int i;

    struct ppnfs_block_node_t* block = blockhd->down;

    mpz_t piprod; // The product of all the Pis
    mpz_init_set_ui(piprod, 1);

    while (block != NULL) {
        mpz_mul(piprod, piprod, block->file->pi);
        block = block->down;
    }

    block = blockhd->down;

    while (block != NULL) {
        mpz_div(dv, piprod, block->file->pi);
        mpz_gcdext(g, ri, si, block->file->pi, dv); // pi*ri + dv*si = g = 1
        mpz_mul(ei, si, dv);
        mpz_mul(ei, ei, block->block);
        mpz_add(partition->e, partition->e, ei);

        block = block->down;
    }

    mpz_clear(g);
    mpz_clear(ri);
    mpz_clear(si);
    mpz_clear(ei);
    mpz_clear(dv);
}

// Initialize the partitions
static void
ppnfs_partition_init()
{
    struct ppnfs_partition_t **partition = &ppnfs_partition_head;
    struct ppnfs_block_head_t *blockhd = ppnfs_block_head;

    while (blockhd != NULL) {
        *partition = (struct ppnfs_partition_t*)
            malloc (sizeof(struct ppnfs_partition_t));
        (*partition)->next = NULL;

        mpz_init((*partition)->e);
        crt(*partition, blockhd);

        blockhd = blockhd->next;
        partition = &(*partition)->next;
    }
}

static void
ppnfs_partition_dump()
{
    Log ("Dumping Partitions\n");
    struct ppnfs_partition_t *current = ppnfs_partition_head;
    int index = 0;
    while (current != NULL) {
        gmp_printf ("PARTITION %d: e=%Zx\n", index++, current->e);
        current = current->next;
    }
}

static void
ppnfs_file_attr_dump()
{
    Log ("Dumping File Attributes\n");

    struct ppnfs_metadata_t* mfather, *mchild;
    mfather = ppnfs_metadata_get_root();
    // Dump the attributes
    for (mchild = ppnfs_metadata_get_child(mfather); mchild; mchild
             = ppnfs_metadata_get(mchild->next)) {
        printf ("FILE '%s' : prime=%lu, c=%lu, pi=%s", mchild->d_name,
             mchild->prime, mchild->c, mchild->pi);
    }
}

mpz_t*
ppnfs_grpir_find_e(int idx)
{
    struct ppnfs_partition_t *current = ppnfs_partition_head;

    while (current != NULL) {
        if (idx-- == 0) {
            return &(current->e);
        }
        current = current->next;
    }
    return NULL;
}

int ppnfs_grpir_init()
{
    struct ppnfs_metadata_head_t* mhead
        = (struct ppnfs_metadata_head_t*)ppnfs_metadata;
    mhead->block_bit_nr = BLOCK_BIT_NR;

    struct ppnfs_metadata_t* mfather, *mchild;
    mfather = ppnfs_metadata_get_root();
    if (!mfather) return -ENOENT;

    struct ppnfs_file_t **file = &ppnfs_file_head;

    queue<struct ppnfs_metadata_t*> dirque; // A queue to store the directories
    dirque.push(mfather);

    while (!dirque.empty()) {
        mfather = dirque.front();
        for (mchild = ppnfs_metadata_get_child(mfather); mchild; mchild
                 = ppnfs_metadata_get(mchild->next)) {

            Log ( "READDIR '%s' (%p, next=%llu)\n", mchild->d_name, mchild, mchild->next );

            if (S_ISDIR( mchild->st.st_mode )) {
                dirque.push(mchild);
                continue;
            }

            *file = ppnfs_file_alloc();
            (*file)->mdata = mchild;

            // Generate attributes @prime, @pi and @c for the current file
            ppnfs_prime_gen(*file); // Generate a @prime for the current file
            ppnfs_pi_gen(*file);    // Generate a @pi for the current file

            ppnfs_metafile_attr_init(mchild, *file);

            // Reading blocks from a specific file
            ppnfs_block_read(mfather, mchild, *file);

            file = &(*file)->next;
        }
        dirque.pop();
    }

    // Building the block tree
    ppnfs_block_tree_build();

#ifdef DEBUG
    ppnfs_block_tree_dump();
#endif

    ppnfs_partition_init();
#ifdef DEBUG
    ppnfs_partition_dump();
#endif

#ifdef DEBUG
    ppnfs_file_attr_dump();
#endif
}
