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

unsigned int NR_BITPERBLK;

void
ppnfs_client_grpir_init()
{
    struct ppnfs_metadata_head_t* head = (struct ppnfs_metadata_head_t*)ppnfs_metadata;
    NR_BITPERBLK = head->bit_per_blk;
}

static void
get_rand_file( void* buf, int len, char* file )
{
	FILE* fp;
	void* p;
	
	fp = fopen(file, "r");
	
	p = buf;
	while( len ) {
		size_t s;
		s = fread(p, 1, len, fp);
		p += s;
		len -= s;
	}
	
	fclose(fp);
}

static void
get_rand_devrandom( void* buf, int len )
{
	get_rand_file(buf, len, "/dev/urandom");
}

void
_init_rand( gmp_randstate_t rand, get_rand_t get_rand, mpz_t seed, int bytes )
{
	void* buf;
	
	buf = malloc(bytes);
	get_rand(buf, bytes);
	gmp_randinit_default(rand);
	mpz_import(seed, bytes, 1, 1, 0, 0, buf);
	gmp_randseed(rand, seed);
	free(buf);
}

void
init_rand( gmp_randstate_t rand, mpz_t seed, int bytes )
{
    _init_rand(rand, get_rand_devrandom, seed, bytes);
}

static void
RandomNum(mpz_t r, int nbits, gmp_randstate_t rand)
{
    do
	{
        mpz_urandomb(r, rand, nbits);
	}while ( !mpz_tstbit(r, nbits - 1) );
}

static void
GeneratePrime2(mpz_t nextprime, mpz_t pos)
{
    do
        mpz_nextprime(nextprime, pos);
    while( !mpz_probab_prime_p(nextprime, 10) );
}

void GenerateQ0(mpz_t Q0, mpz_t q0, mpz_t pi, int nbits, gmp_randstate_t rand)
{
    //counts the #bits of pi
    int pibits = 0;
    int start = NR_BITPERBLK;
    for (;;) {
        int index = mpz_scan1(pi, start); /* pi must be at leat l+1 bits */
        start = index+1;
        if (index == ULONG_MAX)
            break;
        pibits = index + 1;
	}
    //generate a prime of n-1-pibits - why?
    RandomNum(q0, nbits-1-pibits, rand);
    do {
        GeneratePrime2(q0, q0);
        mpz_mul(Q0, q0, pi);
        mpz_mul_ui(Q0, Q0, 2);
        mpz_add_ui(Q0, Q0, 1);
	} while (!mpz_probab_prime_p(Q0, 10));
}

/* Q1=2q1+1 */
void GenerateQ1(mpz_t Q1, mpz_t q1, int nbits, gmp_randstate_t rand)
{
    mpz_t currentp;
    mpz_init(currentp);
    do {
        mpz_urandomb(currentp, rand, nbits-1);
	} while ( !mpz_tstbit(currentp, nbits - 2) );
	
    do {
        //GeneratePrime(q1, nbits-1, rand);
        GeneratePrime2(q1, currentp);
        mpz_set(currentp, q1);
        mpz_mul_ui(Q1, q1, 2);
        mpz_add_ui(Q1, Q1, 1);
	} while (!mpz_probab_prime_p(Q1, 10));
}

/**
 * Generate @g and @q
 */
void
ppnfs_genmodulus(struct ppnfs_metadata_t* mdata, int nbits, gmp_randstate_t rand,
				 mpz_t* g, mpz_t* q, mpz_t* M)
{
    mpz_t q0, q1, Q0, Q1;
	
    mpz_t pi;
    mpz_init(pi);
    gmp_sscanf(mdata->pi, "%Zx\n", &pi);
	
    mpz_init(*q);
    mpz_init(*M);
	
    mpz_init(q0);
    mpz_init(q1);
    mpz_init(Q0);
    mpz_init(Q1);
	
    GenerateQ0(Q0, q0, pi, nbits*3/4, rand);
    GenerateQ1(Q1, q1, nbits/4, rand);
    mpz_mul(*M, Q0, Q1);
	
    //find the generator
    //choose a random g in Zm
    while(1) {
        mpz_t rop;
        mpz_init(*g);
        mpz_init(rop);
        unsigned int rp;
        do {
            mpz_urandomb(*g, rand, nbits);
            mpz_gcd(rop, *g, *M);
            rp = mpz_get_ui(rop);
		} while(rp != 1);
        mpz_mul(*g, *g, *g);
        //test generator
        mpz_t order;
        mpz_init(order);
        mpz_mul(order, q0, pi);
        mpz_mul(order, order, q1);
        mpz_t d[3];
        mpz_init(d[0]);
        mpz_init(d[1]);
        mpz_init(d[2]);
        mpz_mul(d[0], q0, pi);
        mpz_mul(d[1], q1, pi);
        if (mpz_divisible_ui_p(order, mdata->prime)) {
            mpz_divexact_ui(d[2], order, mdata->prime);
		}
        int i;
        int bg = 1;
        for (i = 0; i < 3; i++) {
            mpz_powm(rop, *g, d[i], *M);
            rp = mpz_get_ui(rop);
            if (rp == 1) {
                bg = 0;
                break;
			}
		}
        if (bg == 0)
            continue;
		
        mpz_mul(*q, q0, q1);
		
        break;
	}
}
