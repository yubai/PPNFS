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

static hash_t
sdbm_hash( hash_t h, const char* str, int sz )
{
    int cur = 0;
    const char* c;
    unsigned char d;
    for ( c = str ; *c ; c++ ) {
        d = (unsigned char)(*c);
        h = d + (h<<6) + (h<<16) - h;
        cur++;
        if ( cur == sz ) break;
    }
    return h;
}

#if 1
hash_t continueHashPartial ( hash_t h, const char* str, int sz )
{
    return sdbm_hash( h, str, sz );
}
#else
hash_t continueHashPartial ( hash_t h, const char* str, int sz )
{
    int cur = 0;
    const char* c;
    unsigned char d;
    for ( c = str ; *c ; c++ ) {
        d = (unsigned char)(*c);
		
        h += d + d % 2;
        cur++;
        if ( cur == sz ) break;
    }
    return h % 16;
}
#endif

hash_t continueHash ( hash_t h, const char* str )
{
    return continueHashPartial ( h, str, ~((int)0) );
}

hash_t doHashPartial ( const char* str, int sz )
{
    hash_t h = 0x245;
    return continueHashPartial ( h, str, sz );
}

hash_t doHash ( const char* str )
{
    return doHashPartial ( str, ~((int)0) );
}

static char
ppnfs_client_convert(char ch)
{
    if (ch >= '0' && ch <= '9') {
        ch -= '0';
    } else if (ch >= 'a' && ch <= 'f') {
        ch += 0x0a;
        ch -= 'a';
    }
    return ch;
}

void groupStr(char *dest, char *src)
{
    char high, low;
    char *p;
    char *q;
	
    size_t len = strlen(src);
    int i;
    //if (len < (NR_BITPERBLK >> 3)) {
    //  p = src + len -1;
    //  q = src + (NR_BITPERBLK >> 3) -1;
    //  while (p >= src) {
    //        *q-- = *p--;
    //    }
    //    p = src;
    //    for (i = 0; i < (NR_BITPERBLK >> 3) - len; ++i) {
    //        *p++ = '0';
    //    }
    //}
	
    if ((len % 2) != 0) {
        p = src + len -1;
        q = src + len;
        while (p >= src) {
            *q-- = *p--;
        }
        *src = '0';
    }
	
    p = dest;
    q = src;
	
    while (*q) {
        high = ppnfs_client_convert(*q++);
        low = ppnfs_client_convert(*q++);
        *p++ = (high << 4) + low;
    }
}
