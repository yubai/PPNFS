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
#ifndef _PPNFSNET_H_
#define _PPNFSNET_H_

typedef enum _ppnfs_net_type {
    PPNFS_REQUEST_MHEAD = 1,
    PPNFS_REQUEST_MDATA,
    PPNFS_REQUEST_BLOCK
}ppnfs_net_type;

struct ppnfs_net_cmd {
    ppnfs_net_type type;
};

struct ppnfs_net_mhead_cmd {
    ppnfs_net_type type;
};

struct ppnfs_net_mdata_cmd {
    ppnfs_net_type type;
};

struct ppnfs_net_block_cmd {
    ppnfs_net_type type;
    char g[BLOCK_BIT_NR];
    char M[BLOCK_BIT_NR];
};

void ppnfs_server_net_start();

#endif
