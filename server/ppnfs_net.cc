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

static int
ppnfs_server_net_init(int &hsock, int host_port)
{
    struct sockaddr_in my_addr;
    int * p_int ;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        Err ("Error initializing socket %d\n", errno);
        return -1;
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        Err ("Error setting options %d\n", errno);
        free(p_int);
        return -1;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;

    if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        Err("Error binding to socket, make sure nothing else \
             is listening on this port %d\n",errno);
        return -1;
    }
    if(listen(hsock, 10) == -1 ){
        Err ( "Error listening %d\n", errno );
        return -1;
    }
}

static void*
client_handler(void* arg)
{
    int *newsockfd = (int*)arg;

    char buffer[2048];
    int buffer_len = 1024;
    int bytecount;

    struct ppnfs_net_cmd* cmd;

    while (true) {

        // Reveiving data from client
        bzero(buffer, buffer_len);
        if((bytecount = recv(*newsockfd, buffer, buffer_len, 0))== -1){
            Err ( "---------------------Client disconneted!\n" );
            break;
        }

        if (bytecount == 0) {
            Err ( "---------------------Client disconneted!\n" );
            break;
        }

        Log ( "Received bytes %d\n", bytecount );
        // Analyze the data from the client
        cmd = (struct ppnfs_net_cmd*)buffer;

        switch(cmd->type) {
        case PPNFS_REQUEST_MHEAD: {

            Log ( "-+-+-+MHEAD REQUEST+-+-+-\n" );

            struct ppnfs_metadata_head_t* head = (struct ppnfs_metadata_head_t*)ppnfs_metadata;

            int len = sizeof(struct ppnfs_metadata_head_t);
            if((bytecount = send(*newsockfd, head, len, 0))== -1){
                Err ( "Error sending metadata head %d\n", errno );
                break;
            }

            Log ( "Sent bytes %d\n", bytecount );
            Log ( "Sent metadata head: alloced_nb=%llu, first_free=%llu, block_bit_nr=%lu\n",
                  head->alloced_nb, head->first_free, head->block_bit_nr );

            break;
        }
        case PPNFS_REQUEST_MDATA: {

            Log ( "-+-+-+MDATA REQUEST+-+-+-\n" );

            struct ppnfs_metadata_head_t* head = (struct ppnfs_metadata_head_t*)ppnfs_metadata;

            Log ( "Server metadata structure size=%d\n", sizeof(struct ppnfs_metadata_t) );

            int len = sizeof(struct ppnfs_metadata_t) * head->alloced_nb;

            if((bytecount = send(*newsockfd, head, len, 0))== -1){
                Err ( "Error sending metadata %d\n", errno );
                break;
            }

            Log ( "Sent metadatas: bytes=%d\n", bytecount );
            break;
        }
        case PPNFS_REQUEST_BLOCK: {

            Log ( "-+-+-+BLOCK REQUEST+-+-+-\n" );

            struct ppnfs_net_block_cmd* blkcmd = (struct ppnfs_net_block_cmd*)buffer;

            mpz_t ge;
            mpz_init(ge);

            mpz_t g;
            mpz_t M;
            mpz_init(g);
            mpz_init(M);

            gmp_sscanf(blkcmd->g, "%Zx\n", &g);
            gmp_sscanf(blkcmd->M, "%Zx\n", &M);
            gmp_printf("g=%Zx, M=%Zx\n", g, M);

            int nr;
            if((bytecount = recv(*newsockfd, &nr, sizeof(int), 0))== -1){
                Err ( "Error receiving the block number!\n" );
                break;
            }

            char* gestr = (char*)malloc(sizeof(char) * BLOCK_BIT_NR);
            for (int i = 0; i < nr; ++i) {
                mpz_t* e = ppnfs_grpir_find_e(i);

                mpz_powm(ge, g, *e, M);

            #ifdef DEBUG
                printf("Partition %d GRPIR Attributes:", i);
                gmp_printf("e=%Zx,\tge=%Zx\n", *e, ge);
            #endif

                gmp_sprintf(gestr, "%Zx\n", ge);

                if((bytecount = send(*newsockfd, gestr, BLOCK_BIT_NR, 0))== -1){
                    Err ( "Error sending ge %d\n", errno );
                    break;
                }
            }

            break;
        }
        default: {
            Err ( "Receiving illegal command type.\n" );
            break;
        }
        }
    }

    free(newsockfd);
    return 0;
}

void
ppnfs_server_net_start()
{
    int hsock;
    int host_port= 11010;

    pthread_t thread_id=0;
    socklen_t addr_size = 0;

    int* csock;
    sockaddr_in sadr;

    if (ppnfs_server_net_init(hsock, host_port) == -1) {
        goto FINISH;
    }

    //Now lets do the server stuff
    addr_size = sizeof(sockaddr_in);

    while(true){
        Log ("waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
            Log ("------Received connection from %s------\n",inet_ntoa(sadr.sin_addr));
            pthread_create(&thread_id,0, &client_handler, (void*)csock );
            pthread_detach(thread_id);
        } else{
            Err ( "Error accepting %d\n", errno );
        }
    }

FINISH:
    ;
}
