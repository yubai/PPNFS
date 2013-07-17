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

void ppnfs_config_init()
{
    ppnfs_target = "/home/yub/workspace/thesis/ppnfs/server/testdir";
    ppnfs_metafile = "/home/yub/workspace/thesis/ppnfs/server/PPNFS_METAFILE";
}

int ppnfs_target_init()
{
    int res = 0;
    struct ppnfs_metadata_t* mfather, *mchild;
	
    mfather = ppnfs_metadata_find("/");
    if (!mfather) return -ENOENT;
	
    ppnfs_metadata_release(mfather);
	
    queue<struct ppnfs_metadata_t*> dirque; // A queue to store the directories
    dirque.push(mfather);
	
    while (!dirque.empty()) {
        mfather = dirque.front();
        // The father's id may change by calling the ppnfs_metadata_get_child function
        // Should really be very carefull here!!!
        for (mchild = ppnfs_metadata_get_child(mfather); mchild; mchild
			 = ppnfs_metadata_get(mchild->next)) {
            Log ( "READDIR    '%s' (%p, next=%llu)\n",
				 mchild->d_name, mchild, mchild->next );
			
            if (S_ISDIR( mchild->st.st_mode )) {
                dirque.push(mchild);
                continue;
            }
        }
        dirque.pop();
    }
	
    return res;
}
