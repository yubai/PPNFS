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

#include "ppnfs_client.h"

static int
ppnfs_getattr(const char *path, struct stat *stbuf)
{
    return ppnfs_metadata_getattr(path, stbuf);
}

static int
ppnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *info)
{
    int res = 0;
    struct ppnfs_metadata_t* mfather, *mchild;
    struct stat st;
	
    (void) offset;
    (void) info;
	
    memset(&st, 0, sizeof(struct stat));
    st.st_mode = S_IFDIR;
    res = filler(buf, ".", &st, 0);
    if (res) return res;
    res = filler(buf, "..", &st, 0);
    if (res) return res;
	
    mfather = ppnfs_metadata_find(path);
	
    if (!mfather) return -ENOENT;
	
    for (mchild = ppnfs_metadata_get_child(mfather); mchild; mchild
		 = ppnfs_metadata_get(mchild->next)) {
        res = filler(buf, mchild->d_name, &(mchild->st), 0);
        if (res) break;
    }
	
    ppnfs_metadata_release(mfather);
    return res;
}

static int
ppnfs_open(const char *path, struct fuse_file_info* info)
{
    struct ppnfs_metadata_t* mdata = ppnfs_metadata_find(path);
	
    if (mdata == NULL) {
        return -ENOENT;
    }
    if (!S_ISREG ( mdata->st.st_mode )) {
        ppnfs_metadata_release(mdata);
        return -EISDIR;
    }
	
    ppnfs_metadata_release(mdata);
	
    return 0;
}

static int
ppnfs_read(const char *path, char *buf, size_t size, off_t offset,
		   struct fuse_file_info* info)
{
    struct ppnfs_metadata_t* mdata = ppnfs_metadata_find(path);
	
    clock_t start, end;
	
    start = clock();
	
    char timebuf[128];
	
	
    // int modulusbits = 2048;//128 bytes minimum, to safeguard number field sieve attack
    int modulusbits = NR_BITPERBLK;
    gmp_randstate_t rand;
    mpz_t seed;
    mpz_init(seed);
    init_rand(rand, seed, (modulusbits >> 3) + 1);
	
    mpz_t h, g, q, M;
    ppnfs_genmodulus(mdata, modulusbits, rand, &g, &q, &M);
	
    if (offset < mdata->st.st_size && ppnfs_client_send_gm(&g, &M) == 0) {
        char* pbuf = buf;
        if (offset + size > mdata->st.st_size) {
            size = mdata->st.st_size - offset;
        }
		
        mpz_t he;
        mpz_t data;
		
        unsigned int mask = NR_BITPERBLK >> 4;
		
        unsigned int blk_begin = offset / mask;
        unsigned int blk_end = (offset + size - 1) / mask + 1;
		
        if (ppnfs_client_send_blknr(blk_end - blk_begin) == -1){
            size = 0;
            ppnfs_metadata_release(mdata);
            return size;
        }
		
        char *buffer = malloc((NR_BITPERBLK >> 3) + 1);
        int i;
        for (i = blk_begin; i < blk_end; ++i) {
            mpz_init(h);
            mpz_powm(h, g, q, M);
			
            mpz_t* ge = ppnfs_client_retrieve_block(i);
			
            char* gestr = (char*)malloc(sizeof(char) * NR_BITPERBLK);
            gmp_sprintf(gestr, "%Zx\n", ge);
			
            mpz_init(he);
            mpz_init(data);
            mpz_powm(he, *ge, q, M);
			
            discrete_log(data, h, he, M, mdata->prime, mdata->c);
			
            memset(buffer, 0, (NR_BITPERBLK >> 3) + 1);
            gmp_sprintf(buffer, "%Zx", data);
            groupStr(pbuf, buffer);
            pbuf += (NR_BITPERBLK >> 4);
        }
		
    } else {
        size = 0;
    }
	
    end = clock();
	
    ppnfs_metadata_release(mdata);
	
	return size;
}

static void*
ppnfs_init(struct fuse_conn_info *conn)
{
    Info("Filesystem now serving requests...\n");
    return NULL;
}

struct fuse_operations ppnfs_oper =
{
    .init = ppnfs_init,
    .readdir = ppnfs_readdir,
    .getattr = ppnfs_getattr,
    .open = ppnfs_open,
    .read = ppnfs_read
    /* NULL, // .chmod */
 	/* NULL, // .chown */
 	/* NULL, // .flush */
	/* NULL, // .fsync */
 	/* NULL, // .getattr */
 	/* NULL, // .getdir */
 	/* NULL, // .getxattr */
 	/* NULL, // .link */
	/* NULL, // .listxattr */
 	/* NULL, // .mkdir */
 	/* NULL, // .mknod */
 	/* NULL, // .open */
 	/* NULL, // .read */
 	/* NULL, // .readlink */
 	/* NULL, // .release */
 	/* NULL, // .removexattr */
	/* NULL, // .rename */
 	/* NULL, // .rmdir */
 	/* NULL, // .setxattr */
 	/* NULL, // .statfs */
	/* NULL, // .symlink */
 	/* NULL, // .truncate */
 	/* NULL, // .unlink */
 	/* NULL, // .utime */
 	/* NULL, // .write */
};
