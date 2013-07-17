#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <gmp.h>

#include "config.h"

typedef void (*expaillier_get_rand_t) ( void* buf, int len );

void
expaillier_get_rand_file( void* buf, int len, char* file )
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

void
expaillier_get_rand_devrandom( void* buf, int len )
{
	expaillier_get_rand_file(buf, len, "/dev/urandom");
}

void
init_rand( gmp_randstate_t rand, expaillier_get_rand_t get_rand, mpz_t seed, int bytes )
{
	void* buf;

	buf = malloc(bytes);
	get_rand(buf, bytes);
	gmp_randinit_default(rand);
	mpz_import(seed, bytes, 1, 1, 0, 0, buf);
	gmp_randseed(rand, seed);
	free(buf);
}

int main()
{
    int i, j;
    gmp_randstate_t r_state;
    mpz_t seed;
    mpz_init(seed);

    init_rand(r_state, expaillier_get_rand_devrandom, seed, BIT_PER_LINE >> 3);
    char datafile[30];

    mpz_t block;

    for (i = 0; i < NR_file; ++i) {
        sprintf(datafile, "datafile_%d", i);
        FILE *pfile = fopen(datafile, "w+");
        for (j = 0; j < NR_blk; ++j) {
            mpz_init2(block, BIT_PER_LINE);
            do {
                mpz_urandomb(block, r_state, BIT_PER_LINE);
            } while (!mpz_tstbit(block, BIT_PER_LINE - 1));

            gmp_fprintf(pfile, "%Zx\n", block);
        }
        fclose(pfile);
    }

    gmp_randclear(r_state);
    mpz_clear(block);

    return 0;
}
