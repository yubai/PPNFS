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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <math.h>
#include <gmp.h>
#include <time.h>
#include <sys/time.h>
/* #include "Pohlig_hellman.h" */

/* abase's power only needs to compute once */
int bstored;
struct tagGpow
{
	int index;
	mpz_t gpow;
}arrG[105000];

int comp(const void * a,const void * b)
{
	const struct tagGpow * a1 = (const struct tagGpow * )a;
	const struct tagGpow * b1 = (const struct tagGpow * )b;
	
	return mpz_cmp(a1->gpow, b1->gpow);
}

void precompute(mpz_t g, mpz_t m, int q)
{
	int i;
	int t = (int)sqrt(q);
	
	for (i = 0; i <= (int)q/t; i++)
	{
		arrG[i].index = i;
		mpz_init(arrG[i].gpow);
		mpz_powm_ui(arrG[i].gpow, g, i*t, m);
	}
	qsort(arrG, (int)q/t+1, sizeof(struct tagGpow), comp);
}

int Shank(int* x, mpz_t g, mpz_t y, mpz_t m, int q)
{
	int t = (int)sqrt(q);
	int i;
	mpz_t tmpy;
	if (!bstored)
	{
		precompute(g, m, q);
		bstored = 1;
	}
	mpz_init(tmpy);
	for (i = 0; i <= t; i++)
	{
		mpz_powm_ui(tmpy, g, i, m);
		mpz_mul(tmpy, tmpy, y);
		mpz_mod(tmpy, tmpy, m);
		int mid;
		int min = 0, max = (int)q/t;
		while (max >= min)
		{
			mid = (min + max)/2;
			int r = mpz_cmp(tmpy, arrG[mid].gpow);
			if (r > 0)
				min = mid + 1;
			else if (r < 0)
				max = mid - 1;
			else
			{
				*x = (arrG[mid].index*t-i)%q;
				if (*x < 0)
					*x += q;
				
				return 1;
			}
		}
	}
	return 0;
}


/* order q must be small */
int Shank2(int* x, mpz_t g, mpz_t y, mpz_t m, int q)
{
	int t = (int)sqrt(q);
	int i;
	mpz_t * arrG = (mpz_t*)malloc((int)(q/t+1)*sizeof(mpz_t));
	for (i = 0; i <= (int)q/t; i++)
	{
		mpz_init(arrG[i]);
		mpz_powm_ui(arrG[i], g, i*t, m);
	}
	mpz_t tmpy;
	mpz_init(tmpy);
	for (i = 0; i <= t; i++)
	{
		mpz_powm_ui(tmpy, g, i, m);
		mpz_mul(tmpy, tmpy, y);
		mpz_mod(tmpy, tmpy, m);
		int j;
		for (j = 0; j <= (int)q/t; j++)
		{
			if (!mpz_cmp(tmpy, arrG[j]))
			{
				*x = (j*t-i)%q;
				if (*x < 0)
					*x += q;
				return 1;
			}
		}
	}
	return 0;
}

/* calculate x such that b=a^x mod m */
/* order q = p^c where p is a prime */
void discrete_log(mpz_t x, mpz_t a, mpz_t b, mpz_t m, int p, int c)
{
    int * arrA = (int*) malloc(c*sizeof(int));
    mpz_t * arrP = (mpz_t*)malloc(c*sizeof(mpz_t));
	
	int i;
	for (i = 0; i < c; i++)
	{
		mpz_init(arrP[i]);
		mpz_ui_pow_ui(arrP[i], p, i);
	}
	
	//extract a[0] to a[c-1]
	mpz_t bi[2], abase, tmpy;
	mpz_init(bi[0]);
	mpz_init(bi[1]);
	mpz_init(tmpy);
	mpz_init(abase);
	int pos = 1;
	//compute a[0] first
	mpz_set(bi[0], b);
	mpz_powm(abase, a, arrP[c-1], m);
	
	mpz_powm(tmpy, bi[0], arrP[c-1], m);
	//  gmp_printf("tmpy = %Zx\n", tmpy);
	
	
	precompute(abase, m, p);
	bstored = 1;
	
	if(!Shank(&arrA[0], abase, tmpy, m, p))
	{
		printf("did not find log for %d\n", i);
	}
	
	mpz_set_ui(x, arrA[0]);
	//compute a[1] to a[c-1]
	mpz_t ap;
	mpz_init(ap);
	
	
	for (i = 1; i < c; i++)
	{
		mpz_mul_ui(ap, arrP[i-1], arrA[i-1]);
		mpz_invert(bi[pos], a, m);
		mpz_powm(bi[pos], bi[pos], ap, m);
		mpz_mul(bi[pos], bi[1-pos], bi[pos]);
		mpz_powm(tmpy, bi[pos], arrP[c-i-1], m);
		
		if (!Shank(&arrA[i], abase, tmpy, m, p))
		{
			printf("did not find log for %d\n", i);
		}
		
		pos = 1-pos;
		mpz_mul_ui(tmpy, arrP[i], arrA[i]);
		mpz_add(x, x, tmpy);
	}
}
