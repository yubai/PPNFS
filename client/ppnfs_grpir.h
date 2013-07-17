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

#ifndef _PPNFSGRPIR_H_
#define _PPNFSGRPIR_H_

typedef void (*get_rand_t) ( void* buf, int len );

void ppnfs_grpir_init();

void ppnfs_grpir_set_blk_nb(unsigned long long int file_sz, unsigned int NR_bit);
unsigned long long int ppnfs_grpir_get_blk_nb();

/* void blocks_init(); */
/* int blocks_read(); */
/* void blocks_dump(); */
/* void blocks_release(); */

void init_rand( gmp_randstate_t rand, mpz_t seed, int bytes );

void ppnfs_genmodulus(struct ppnfs_metadata_t* mdata, int nbits,
					  gmp_randstate_t rand, mpz_t* g, mpz_t* q, mpz_t* M);

#endif
