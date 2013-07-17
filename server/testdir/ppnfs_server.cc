#include "ppnfs_server.h"

const char *ppnfs_metafile = NULL;
const char *ppnfs_target = NULL;

int
main(int argc, char* argv[])
{
    // Init the common variables
    ppnfs_config_init();

    // Open the metafile
    ppnfs_metadata_init();

    // Scan the target directory
    ppnfs_target_init();

// //   printf("size=%d\n", sizeof(struct ppnfs_metadata_t));
//     clock_t start, end;
//     start = clock();

//     // for (int i = 0; i < 100; ++i) {
//         struct ppnfs_metadata_t* mdata;
//         // mdata = ppnfs_metadata_find(
//         //     "/firstlevel_0/secondlevel_0");
//         // ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_0/secondlevel_1/thirdlevel_2/fourthlevel_3/fifthlevel_4");
//         ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_0/secondlevel_2/thirdlevel_3/fourthlevel_4/fifthlevel_4");
//         ppnfs_metadata_release(mdata);

//         mdata = ppnfs_metadata_find(
//             "/firstlevel_1/secondlevel_2/thirdlevel_0/fourthlevel_0/fifthlevel_1");
//         ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_1/secondlevel_4/thirdlevel_1/fourthlevel_3/fifthlevel_4");
//         ppnfs_metadata_release(mdata);

//         mdata = ppnfs_metadata_find(
//             "/firstlevel_2/secondlevel_2/thirdlevel_0/fourthlevel_0/fifthlevel_1");
//         ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_2/secondlevel_4/thirdlevel_2/fourthlevel_3/fifthlevel_3");
//         ppnfs_metadata_release(mdata);

//         mdata = ppnfs_metadata_find(
//             "/firstlevel_3/secondlevel_2/thirdlevel_0/fourthlevel_0/fifthlevel_1");
//         ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_3/secondlevel_3/thirdlevel_1/fourthlevel_4/fifthlevel_2");
//         ppnfs_metadata_release(mdata);

//         mdata = ppnfs_metadata_find(
//             "/firstlevel_4/secondlevel_2/thirdlevel_0/fourthlevel_0/fifthlevel_1");
//         ppnfs_metadata_release(mdata);
//         mdata = ppnfs_metadata_find(
//             "/firstlevel_4/secondlevel_4/thirdlevel_0/fourthlevel_1/fifthlevel_3");
//         ppnfs_metadata_release(mdata);
//     // }
//     end = clock();
//     // if (!mdata) return -ENOENT;

//     // printf ("file name: %s, searching time=%ld\n", mdata->d_name, start - end);
//     printf ("searching time=%ld\n",end - start);

//     start = clock();
//     int res = 0, j=-1;
//     struct ppnfs_metadata_t* mfather, *mchild;

//     mfather = ppnfs_metadata_find("/");
//     if (!mfather) return -ENOENT;

//     ppnfs_metadata_release(mfather);

// search:
//     j++;
//     for (mchild = ppnfs_metadata_get_child(mfather); mchild; mchild
//              = ppnfs_metadata_get(mchild->next)) {
//         // printf ( "READDIR    '%s' (%p, next=%llu)\n",
//         //           mchild->d_name, mchild, mchild->next );
//         if (j == 0) {
//             if (strcmp(mchild->d_name, "firstlevel_0") == 0) {
//                 mfather = mchild;
//                 goto search;
//             }
//             continue;
//         } else if (j == 1) {
//             if (strcmp(mchild->d_name, "secondlevel_1") == 0) {
//                 mfather = mchild;
//                 goto search;
//             }
//             continue;
//         } else if (j == 2) {
//             if (strcmp(mchild->d_name, "thirdlevel_2") == 0) {
//                 mfather = mchild;
//                 goto search;
//             }
//             continue;
//         } else if (j == 3) {
//             if (strcmp(mchild->d_name, "fourthlevel_3") == 0) {
//                 mfather = mchild;
//                 goto search;
//             }
//             continue;
//         } else if (j == 4) {
//             if (strcmp(mchild->d_name, "fifththlevel_4") == 0) {
//                 mfather = mchild;
//                 goto found;
//             }
//             continue;
//         }
//     }
// found:
//     end = clock();
//     printf ("file name:%s, searching time=%ld\n",mfather->d_name, end - start);
    // GR-PIR variables initialization
    ppnfs_grpir_init();

    // start servering requtests
    ppnfs_server_net_start();

    return 0;
}
