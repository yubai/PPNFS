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

int hsock;
ssize_t bytecount;

size_t ppnfs_metadata_struct_size = 0;

static int
ppnfs_client_net_send(int* hsock, struct ppnfs_net_cmd* cmd)
{
    char buffer[1024];
    int bytecount;
    int cmd_sz;

    switch(cmd->type) {
    case PPNFS_REQUEST_MHEAD:
        cmd_sz = sizeof(struct ppnfs_net_mhead_cmd);
        break;
    case PPNFS_REQUEST_MDATA:
        cmd_sz = sizeof(struct ppnfs_net_mdata_cmd);
        break;
    case PPNFS_REQUEST_BLOCK:
        cmd_sz = sizeof(struct ppnfs_net_block_cmd) + (NR_BITPERBLK << 1);
        break;
    default:
        return -1;
    }

    if( (bytecount=send(*hsock, cmd, cmd_sz, 0))== -1){
        Err ("Error sending data %d\n", errno);
        return -1;
    }

    return 0;
}

static int
ppnfs_client_net_init(int* hsock, char* host_name, int host_port)
{
    int *p_int;
    struct sockaddr_in my_addr;
    int err;

    *hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(*hsock == -1){
        Err ("Error initializing socket %d\n",errno);
        return -1;
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(*hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(*hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        Err ( "Error setting options %d\n", errno );
        free(p_int);
        return -1;
    }

    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name);

    if( connect( *hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            Err ( "Error connecting socket %d\n", errno);
            return -1;
        }
    }

}

void
ppnfs_client_net_close()
{
    close(hsock);
}

struct ppnfs_net_cmd*
mheadcmd()
{
    struct ppnfs_net_mhead_cmd* cmd;
    cmd = (struct ppnfs_net_mhead_cmd*) malloc (sizeof(struct ppnfs_net_mhead_cmd));
    cmd->type = PPNFS_REQUEST_MHEAD;
    return (struct ppnfs_net_cmd*)cmd;
}

struct ppnfs_net_cmd*
mdatacmd()
{
    struct ppnfs_net_mdata_cmd* cmd;
    cmd = (struct ppnfs_net_mdata_cmd*) malloc (sizeof(struct ppnfs_net_mdata_cmd));
    cmd->type = PPNFS_REQUEST_MDATA;
    return (struct ppnfs_net_cmd*)cmd;
}

struct ppnfs_net_cmd*
blockcmd(mpz_t* g, mpz_t* M/* , unsigned int partition_idx */)
{
    struct ppnfs_net_block_cmd* cmd;
    // The size of g or M is as long as the size of NR_BITPERBLK
    cmd = (struct ppnfs_net_block_cmd*)
        malloc (sizeof(struct ppnfs_net_block_cmd) + (NR_BITPERBLK << 2));
    cmd->type = PPNFS_REQUEST_BLOCK;
    gmp_sprintf(cmd->gm, "%Zx\n", *g);
    gmp_sprintf(((char*)(cmd->gm) + NR_BITPERBLK), "%Zx\n", *M);
    return (struct ppnfs_net_cmd*)cmd;
}

void ppnfs_client_net_start()
{

    int host_port = 11010;
    char* host_name = "127.0.0.1"; // server address

    if (ppnfs_client_net_init(&hsock, host_name, host_port) == -1) {
        goto FINISH;
    }

    //--------------------------------------------------------------------------
    // Retrieving the metadata head structure first

    struct ppnfs_net_cmd* cmd = mheadcmd();
    if (ppnfs_client_net_send(&hsock,  cmd) == -1) {
        Err ("Error sending data %d\n", errno);
        goto FINISH;
    }
    free (cmd);

    struct ppnfs_metadata_head_t* head =
        malloc ( sizeof(struct ppnfs_metadata_head_t) );

    if((bytecount = recv(hsock, head,
                         sizeof(struct ppnfs_metadata_head_t), 0))== -1){
        Err ( "Error receiving metadata head %d\n", errno);
        goto FINISH;
    }

    NR_BITPERBLK = head->bit_per_blk;

#ifdef DEBUG
    Log ( "Received bytes %d\n", bytecount );
    Log ( "Received metadata head structure: alloced_nb=%llu, first_free=%llu, bit_per_blk=%d\n",
          head->alloced_nb, head->first_free, NR_BITPERBLK/* head->bit_per_blk */ );
#endif

    //--------------------------------------------------------------------------------
    // Now retrieving the metadata

    cmd = mdatacmd();
    if (ppnfs_client_net_send(&hsock,  cmd) == -1) {
        Err ("Error sending data %d\n", errno);
        goto FINISH;
    }
    free (cmd);

    // The real size of ppnfs_metadata type
    ppnfs_metadata_struct_size =
        sizeof(struct ppnfs_metadata_t) + sizeof(char) * ((NR_BITPERBLK>>3) + 1);

    // structures are aligned at boundaries.
    // For 32-bit machine: usually of 4 bytes(although it can be changed)
    // For 64-bit machine: usually of 8 bytes
    // structure has padding
    ppnfs_metadata_struct_size = (ppnfs_metadata_struct_size + 7) & (~0x07);

#ifdef DEBUG
    Log ( "Client metadata structure2 size=%d\n", sizeof(struct ppnfs_metadata_t) );
    Log ( "Client metadata structure size=%d\n", ppnfs_metadata_struct_size );
#endif


    struct ppnfs_metadata_t* ppnfs_metadata =
        malloc ( ppnfs_metadata_struct_size * head->alloced_nb );

#ifdef DEBUG
    Log ( "buffer size=%d\n", ppnfs_metadata_struct_size * head->alloced_nb);
#endif

    // Since the size of read buffer is limited, I use the while loop to receive
    // more bytes the buffer can hold.
    int i;
    for (i = 0; i < ppnfs_metadata_struct_size * head->alloced_nb; ++i) {
        if (recv(hsock, (char*)ppnfs_metadata + i, 1, 0) == -1) {
            Err ( "Error receiving metadata %d\n", errno);
            goto FINISH;
        }
    }

    // // DO NOT work! The total size received is more than the size of the buffer
    // if((bytecount = recv(hsock, ppnfs_metadata,
    //                      ppnfs_metadata_struct_size * head->alloced_nb, 0))== -1){
    //     Err ( "Error receiving metadata %d\n", errno);
    //     goto FINISH;
    // }

#ifdef DEBUG
    Log ( "Recieved metadata: bytes=%d\n", i);
#endif

    //--------------------------------------------------------------------------
    // Write the metadata into local disk

    int ppnfs_metadata_fd;

    ppnfs_metadata_fd =
        open(ppnfs_metafile, O_CREAT | O_TRUNC | O_RDWR,
            (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH));

    if ( ppnfs_metadata_fd == -1 ) {
        Err ( "Could not open '%s' : err=%d:%s\n",
        ppnfs_metafile, errno, strerror(errno));
    }

    pwrite(ppnfs_metadata_fd, ppnfs_metadata,
        ppnfs_metadata_struct_size * head->alloced_nb, 0);

    free (head);
    free (ppnfs_metadata);

    close(ppnfs_metadata_fd);

FINISH:
    ;
}

int
ppnfs_client_send_gm(mpz_t* g, mpz_t* M)
{
    int i;
    struct ppnfs_net_cmd* cmd = blockcmd(g, M);

    if (ppnfs_client_net_send(&hsock,  cmd) == -1) {
        Err ("Error sending gm %d\n", errno);
        return -1;
    }
    free (cmd);

    return 0;
}

int
ppnfs_client_send_blknr(int nr)
{
    if( (bytecount=send(hsock, &nr, sizeof(int), 0))== -1){
        Err ("Error sending data %d\n", errno);
        return -1;
    }
    return 0;
}

mpz_t*
ppnfs_client_retrieve_block(unsigned int idx)
{
    char* gestr = malloc(sizeof(char) * NR_BITPERBLK);
    mpz_t* ge = (mpz_t*)malloc(sizeof(mpz_t));

    if((bytecount = recv(hsock, gestr, NR_BITPERBLK, 0))== -1){
        Err ( "Error receiving ge %d\n", errno);
    }

#ifdef DEBUG
    Log ( "Received bytes %d\n", bytecount );
    Log ("gestr: %s\n", gestr);
#endif

    mpz_init(*ge);
    gmp_sscanf(gestr, "%Zx\n", ge);

    return ge;
}
