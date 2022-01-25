/******************************************************************************

                  ???? (C), 2001-2011, ????????

 ******************************************************************************
  ? ? ?   : hmac_main.c
  ? ? ?   : ??
  ?    ?   : ???
  ????   : 2012?9?18?
  ????   :
  ????   : HMAC????????
  ????   :
  ????   :
  1.?    ?   : 2012?9?18?
    ?    ?   : ???
    ????   : ????

******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

/*****************************************************************************
  1 ?????
*****************************************************************************/
#include "hmac_cali_mgmt.h"
#include "oal_kernel_file.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_C

extern oal_uint32 wlan_pm_close(oal_void);
/*  大小:16bit * 11 * 32 = 704 Byte */
oal_int16 g_as_rxiq_h_comp_80M[HI1102_CALI_RXIQ_LS_FILTER_TAP_NUM][HI1102_CALI_RXIQ_LS_FILTER_FEQ_NUM_80M] =
{
    {-374,417,368,-122,-2570,6130,-6916,3066,3066,-6916,6130,-2570,-122,368,417,-374,-645,-472,801,681,-2266,1249,2617,-6233,6233,-2617,-1249,2266,-681,-801,472,645,},
    {1099,-833,-1320,-318,4962,-5797,2158,49,49,2158,-5797,4962,-318,-1320,-833,1099,1369,1290,-1171,-1260,-37,4865,-6820,5154,-5154,6820,-4865,37,1260,1171,-1290,-1369,},
    {-1855,716,1856,823,-288,-3149,577,1319,1319,577,-3149,-288,823,1856,716,-1855,-1442,-1944,5,634,3558,-3042,1276,-4784,4784,-1276,3042,-3558,-634,-5,1944,1442,},
    {2150,329,-878,-585,-4493,2435,-1885,2928,2928,-1885,2435,-4493,-585,-878,329,2150,808,1941,1807,726,-328,-2940,-245,-2808,2808,245,2940,328,-726,-1807,-1941,-808,},
    {-1973,-1777,-1102,-326,200,1405,1474,2099,2099,1474,1405,200,-326,-1102,-1777,-1973,-190,-1195,-1998,-1120,-3712,913,-4554,3487,-3487,4554,-913,3712,1120,1998,1195,190,},
    {1811,2466,2135,849,4435,-905,4497,1098,1098,4497,-905,4435,849,2135,2466,1811,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {-1973,-1777,-1102,-326,200,1405,1474,2099,2099,1474,1405,200,-326,-1102,-1777,-1973,190,1195,1998,1120,3712,-913,4554,-3487,3487,-4554,913,-3712,-1120,-1998,-1195,-190,},
    {2150,329,-878,-585,-4493,2435,-1885,2928,2928,-1885,2435,-4493,-585,-878,329,2150,-808,-1941,-1807,-726,328,2940,245,2808,-2808,-245,-2940,-328,726,1807,1941,808,},
    {-1855,716,1856,823,-288,-3149,577,1319,1319,577,-3149,-288,823,1856,716,-1855,1442,1944,-5,-634,-3558,3042,-1276,4784,-4784,1276,-3042,3558,634,5,-1944,-1442,},
    {1099,-833,-1320,-318,4962,-5797,2158,49,49,2158,-5797,4962,-318,-1320,-833,1099,-1369,-1290,1171,1260,37,-4865,6820,-5154,5154,-6820,4865,-37,-1260,-1171,1290,1369,},
    {-374,417,368,-122,-2570,6130,-6916,3066,3066,-6916,6130,-2570,-122,368,417,-374,645,472,-801,-681,2266,-1249,-2617,6233,-6233,2617,1249,-2266,681,801,-472,-645,},
};
/*  大小:16bit * 11 * 64 = 1408 Byte */
oal_int16 g_as_rxiq_h_comp_160M[HI1102_CALI_RXIQ_LS_FILTER_TAP_NUM][HI1102_CALI_RXIQ_LS_FILTER_FEQ_NUM_160M] =
{
    {-268,-102,124,276,281,168,35,-36,-2897,1138,2949,940,-2121,-2700,-251,2462,2462,-251,-2700,-2121,940,2949,1138,-2897,-36,35,168,281,276,124,-102,-268,-140,-325,-292,-69,191,330,292,145,1090,-3567,-706,3009,2647,-975,-3397,-1842,1842,3397,975,-2647,-3009,706,3567,-1090,-145,-292,-330,-191,69,292,325,140,},
    {715,342,-197,-618,-741,-580,-294,-62,3004,1547,-2165,-2598,-211,1534,900,-575,-575,900,1534,-211,-2598,-2165,1547,3004,-62,-294,-580,-741,-618,-197,342,715,289,697,708,351,-124,-436,-458,-267,-4887,5635,3809,-1968,-3756,-779,2193,1460,-1460,-2193,779,3756,1968,-3809,-5635,4887,267,458,436,124,-351,-708,-697,-289,},
    {-1113,-646,48,638,894,793,491,191,1645,-2822,-1227,160,-206,-593,268,1480,1480,268,-593,-206,160,-1227,-2822,1645,191,491,793,894,638,48,-646,-1113,-333,-847,-996,-775,-368,-12,153,136,4553,-1515,-1705,486,885,-964,-2360,-1256,1256,2360,964,-885,-486,1705,1515,-4553,-136,-153,12,368,775,996,847,333,},
    {1186,880,403,-49,-325,-383,-287,-145,-3919,383,505,-341,-404,261,948,1286,1286,948,261,-404,-341,505,383,-3919,-145,-287,-383,-325,-49,403,880,1186,239,663,939,1017,905,664,385,154,1846,-2556,-1707,-45,74,-995,-1595,-779,779,1595,995,-74,45,1707,2556,-1846,-154,-385,-664,-905,-1017,-939,-663,-239,},
    {-987,-981,-940,-830,-646,-423,-217,-73,-491,994,487,273,733,1166,1107,828,828,1107,1166,733,273,487,994,-491,-73,-217,-423,-646,-830,-940,-981,-987,-103,-322,-553,-744,-815,-721,-497,-240,-5228,2118,937,-2201,-2629,-390,1492,950,-950,-1492,390,2629,2201,-937,-2118,5228,240,497,721,815,744,553,322,103,},
    {854,1000,1183,1255,1132,840,489,202,4089,-701,134,1509,1576,1021,823,977,977,823,1021,1576,1509,134,-701,4089,202,489,840,1132,1255,1183,1000,854,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {-987,-981,-940,-830,-646,-423,-217,-73,-491,994,487,273,733,1166,1107,828,828,1107,1166,733,273,487,994,-491,-73,-217,-423,-646,-830,-940,-981,-987,103,322,553,744,815,721,497,240,5228,-2118,-937,2201,2629,390,-1492,-950,950,1492,-390,-2629,-2201,937,2118,-5228,-240,-497,-721,-815,-744,-553,-322,-103,},
    {1186,880,403,-49,-325,-383,-287,-145,-3919,383,505,-341,-404,261,948,1286,1286,948,261,-404,-341,505,383,-3919,-145,-287,-383,-325,-49,403,880,1186,-239,-663,-939,-1017,-905,-664,-385,-154,-1846,2556,1707,45,-74,995,1595,779,-779,-1595,-995,74,-45,-1707,-2556,1846,154,385,664,905,1017,939,663,239,},
    {-1113,-646,48,638,894,793,491,191,1645,-2822,-1227,160,-206,-593,268,1480,1480,268,-593,-206,160,-1227,-2822,1645,191,491,793,894,638,48,-646,-1113,333,847,996,775,368,12,-153,-136,-4553,1515,1705,-486,-885,964,2360,1256,-1256,-2360,-964,885,486,-1705,-1515,4553,136,153,-12,-368,-775,-996,-847,-333,},
    {715,342,-197,-618,-741,-580,-294,-62,3004,1547,-2165,-2598,-211,1534,900,-575,-575,900,1534,-211,-2598,-2165,1547,3004,-62,-294,-580,-741,-618,-197,342,715,-289,-697,-708,-351,124,436,458,267,4887,-5635,-3809,1968,3756,779,-2193,-1460,1460,2193,-779,-3756,-1968,3809,5635,-4887,-267,-458,-436,-124,351,708,697,289,},
    {-268,-102,124,276,281,168,35,-36,-2897,1138,2949,940,-2121,-2700,-251,2462,2462,-251,-2700,-2121,940,2949,1138,-2897,-36,35,168,281,276,124,-102,-268,140,325,292,69,-191,-330,-292,-145,-1090,3567,706,-3009,-2647,975,3397,1842,-1842,-3397,-975,2647,3009,-706,-3567,1090,145,292,330,191,-69,-292,-325,-140,},
};
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
/*  大小:16bit * 11 * 128 = 2816 Byte */
oal_int16 g_as_txiq_h_comp_320M[HI1102_CALI_TXIQ_LS_FILTER_TAP_NUM][HI1102_CALI_TXIQ_LS_FILTER_FEQ_NUM_320M] =
{
    {-262,-208,-111,9,127,218,264,256,197,99,-17,-128,-213,-256,-251,-203,-123,-29,60,126,161,162,135,90,3969,-220,-2760,-3356,-2287,-272,1779,3044,3044,1779,-272,-2287,-3356,-2760,-220,3969,90,135,162,161,126,60,-29,-123,-203,-251,-256,-213,-128,-17,99,197,256,264,218,127,9,-111,-208,-262,-64,-179,-257,-282,-250,-166,-49,78,190,263,286,253,171,58,-66,-178,-261,-304,-303,-265,-203,-131,-65,-15,1288,1988,1157,-363,-1728,-2327,-1938,-748,748,1938,2327,1728,363,-1157,-1988,-1288,15,65,131,203,265,303,304,261,178,66,-58,-171,-253,-286,-263,-190,-78,49,166,250,282,257,179,64,},
    {545,455,290,79,-141,-333,-466,-519,-489,-383,-224,-40,138,283,378,414,396,335,250,159,79,19,-16,-28,-2360,-1144,101,872,998,619,61,-327,-327,61,619,998,872,101,-1144,-2360,-28,-16,19,79,159,250,335,396,414,378,283,138,-40,-224,-383,-489,-519,-466,-333,-141,79,290,455,545,123,347,510,584,557,437,248,23,-195,-371,-477,-500,-443,-321,-161,7,154,259,313,315,276,211,137,71,2169,-669,-1636,-1339,-526,172,413,200,-200,-413,-172,526,1339,1636,669,-2169,-71,-137,-211,-276,-315,-313,-259,-154,-7,161,321,443,500,477,371,195,-23,-248,-437,-557,-584,-510,-347,-123,},
    {-550,-497,-397,-260,-100,67,224,356,450,498,498,452,368,258,136,15,-90,-169,-218,-234,-221,-186,-138,-87,-4089,-610,1444,2105,1688,684,-370,-1025,-1025,-370,684,1688,2105,1444,-610,-4089,-87,-138,-186,-221,-234,-218,-169,-90,15,136,258,368,452,498,498,450,356,224,67,-100,-260,-397,-497,-550,-80,-233,-364,-461,-516,-524,-484,-401,-284,-143,6,150,276,373,434,458,445,402,338,262,186,116,60,20,-319,-1249,-1208,-671,-60,333,391,167,-167,-391,-333,60,671,1208,1249,319,-20,-60,-116,-186,-262,-338,-402,-445,-458,-434,-373,-276,-150,-6,143,284,401,484,524,516,461,364,233,80,},
    {472,462,440,403,350,279,190,85,-31,-151,-266,-368,-449,-503,-527,-520,-487,-432,-362,-287,-212,-145,-89,-46,-1592,186,1008,1155,904,499,125,-92,-92,125,499,904,1155,1008,186,-1592,-46,-89,-145,-212,-287,-362,-432,-487,-520,-527,-503,-449,-368,-266,-151,-31,85,190,279,350,403,440,462,472,33,102,177,259,347,432,507,560,582,567,512,422,307,178,50,-62,-148,-202,-223,-214,-183,-141,-96,-56,-2585,-753,63,182,-21,-230,-266,-115,115,266,230,21,-182,-63,753,2585,56,96,141,183,214,223,202,148,62,-50,-178,-307,-422,-512,-567,-582,-560,-507,-432,-347,-259,-177,-102,-33,},
    {-505,-500,-492,-480,-464,-444,-420,-391,-357,-316,-269,-216,-159,-101,-43,8,52,83,102,107,101,85,64,42,2183,690,-169,-405,-159,344,857,1173,1173,857,344,-159,-405,-169,690,2183,42,64,85,101,107,102,83,52,8,-43,-101,-159,-216,-269,-316,-357,-391,-420,-444,-464,-480,-492,-500,-505,-38,-112,-175,-223,-256,-276,-290,-303,-322,-353,-396,-447,-502,-551,-585,-596,-579,-535,-467,-382,-290,-201,-124,-64,-2386,-221,521,396,-48,-395,-437,-185,185,437,395,48,-396,-521,221,2386,64,124,201,290,382,467,535,579,596,585,551,502,447,396,353,322,303,290,276,256,223,175,112,38,},
    {542,536,525,511,496,484,477,477,484,498,518,539,559,572,575,563,536,492,434,365,290,215,145,85,3915,842,-756,-1119,-621,291,1187,1728,1728,1187,291,-621,-1119,-756,842,3915,85,145,215,290,365,434,492,536,563,575,572,559,539,518,498,484,477,477,484,496,511,525,536,542,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {-505,-500,-492,-480,-464,-444,-420,-391,-357,-316,-269,-216,-159,-101,-43,8,52,83,102,107,101,85,64,42,2183,690,-169,-405,-159,344,857,1173,1173,857,344,-159,-405,-169,690,2183,42,64,85,101,107,102,83,52,8,-43,-101,-159,-216,-269,-316,-357,-391,-420,-444,-464,-480,-492,-500,-505,38,112,175,223,256,276,290,303,322,353,396,447,502,551,585,596,579,535,467,382,290,201,124,64,2386,221,-521,-396,48,395,437,185,-185,-437,-395,-48,396,521,-221,-2386,-64,-124,-201,-290,-382,-467,-535,-579,-596,-585,-551,-502,-447,-396,-353,-322,-303,-290,-276,-256,-223,-175,-112,-38,},
    {472,462,440,403,350,279,190,85,-31,-151,-266,-368,-449,-503,-527,-520,-487,-432,-362,-287,-212,-145,-89,-46,-1592,186,1008,1155,904,499,125,-92,-92,125,499,904,1155,1008,186,-1592,-46,-89,-145,-212,-287,-362,-432,-487,-520,-527,-503,-449,-368,-266,-151,-31,85,190,279,350,403,440,462,472,-33,-102,-177,-259,-347,-432,-507,-560,-582,-567,-512,-422,-307,-178,-50,62,148,202,223,214,183,141,96,56,2585,753,-63,-182,21,230,266,115,-115,-266,-230,-21,182,63,-753,-2585,-56,-96,-141,-183,-214,-223,-202,-148,-62,50,178,307,422,512,567,582,560,507,432,347,259,177,102,33,},
    {-550,-497,-397,-260,-100,67,224,356,450,498,498,452,368,258,136,15,-90,-169,-218,-234,-221,-186,-138,-87,-4089,-610,1444,2105,1688,684,-370,-1025,-1025,-370,684,1688,2105,1444,-610,-4089,-87,-138,-186,-221,-234,-218,-169,-90,15,136,258,368,452,498,498,450,356,224,67,-100,-260,-397,-497,-550,80,233,364,461,516,524,484,401,284,143,-6,-150,-276,-373,-434,-458,-445,-402,-338,-262,-186,-116,-60,-20,319,1249,1208,671,60,-333,-391,-167,167,391,333,-60,-671,-1208,-1249,-319,20,60,116,186,262,338,402,445,458,434,373,276,150,6,-143,-284,-401,-484,-524,-516,-461,-364,-233,-80,},
    {545,455,290,79,-141,-333,-466,-519,-489,-383,-224,-40,138,283,378,414,396,335,250,159,79,19,-16,-28,-2360,-1144,101,872,998,619,61,-327,-327,61,619,998,872,101,-1144,-2360,-28,-16,19,79,159,250,335,396,414,378,283,138,-40,-224,-383,-489,-519,-466,-333,-141,79,290,455,545,-123,-347,-510,-584,-557,-437,-248,-23,195,371,477,500,443,321,161,-7,-154,-259,-313,-315,-276,-211,-137,-71,-2169,669,1636,1339,526,-172,-413,-200,200,413,172,-526,-1339,-1636,-669,2169,71,137,211,276,315,313,259,154,7,-161,-321,-443,-500,-477,-371,-195,23,248,437,557,584,510,347,123,},
    {-262,-208,-111,9,127,218,264,256,197,99,-17,-128,-213,-256,-251,-203,-123,-29,60,126,161,162,135,90,3969,-220,-2760,-3356,-2287,-272,1779,3044,3044,1779,-272,-2287,-3356,-2760,-220,3969,90,135,162,161,126,60,-29,-123,-203,-251,-256,-213,-128,-17,99,197,256,264,218,127,9,-111,-208,-262,64,179,257,282,250,166,49,-78,-190,-263,-286,-253,-171,-58,66,178,261,304,303,265,203,131,65,15,-1288,-1988,-1157,363,1728,2327,1938,748,-748,-1938,-2327,-1728,-363,1157,1988,1288,-15,-65,-131,-203,-265,-303,-304,-261,-178,-66,58,171,253,286,263,190,78,-49,-166,-250,-282,-257,-179,-64,},
};
#else
/*  大小:16bit * 11 * 64 = 1408 Byte */
oal_int16 g_as_txiq_h_comp_160M[HI1102_CALI_TXIQ_LS_FILTER_TAP_NUM][HI1102_CALI_TXIQ_LS_FILTER_FEQ_NUM_160M] =
{
    {-499,-104,371,553,304,-171,-507,-468,-121,251,391,264,4425,-7033,-3026,5368,5368,-3026,-7033,4425,264,391,251,-121,-468,-507,-171,304,553,371,-104,-499,-249,-553,-427,29,464,552,237,-248,-577,-580,-340,-81,3386,791,-4189,-2767,2767,4189,-791,-3386,81,340,580,577,248,-237,-552,-464,-29,427,553,249,},
    {1026,375,-491,-1012,-888,-259,439,807,728,390,73,-59,-3821,1328,1823,-458,-458,1823,1328,-3821,-59,73,390,728,807,439,-259,-888,-1012,-491,375,1026,484,1126,1022,274,-589,-1009,-781,-145,445,667,517,223,1786,-3210,-599,528,-528,599,3210,-1786,-223,-517,-667,-445,145,781,1009,589,-274,-1022,-1126,-484,},
    {-1044,-665,-49,570,959,980,654,152,-295,-513,-466,-259,-5280,4277,2764,-1786,-1786,2764,4277,-5280,-259,-466,-513,-295,152,654,980,959,570,-49,-665,-1044,-316,-833,-1051,-895,-432,157,655,902,857,608,306,82,-1548,-1915,241,547,-547,-241,1915,1548,-82,-306,-608,-857,-902,-655,-157,432,895,1051,833,316,},
    {945,847,625,269,-182,-629,-947,-1052,-937,-677,-383,-149,-1647,2473,1572,-129,-129,1572,2473,-1647,-149,-383,-677,-937,-1052,-947,-629,-182,269,625,847,945,131,428,775,1075,1166,951,489,-25,-376,-464,-345,-162,-3578,410,-45,-300,300,45,-410,3578,162,345,464,376,25,-489,-951,-1166,-1075,-775,-428,-131,},
    {-1013,-973,-902,-805,-674,-496,-271,-35,152,238,213,121,3138,-917,3,2221,2221,3,-917,3138,121,213,238,152,-35,-271,-496,-674,-805,-902,-973,-1013,-155,-409,-540,-590,-665,-832,-1052,-1194,-1139,-875,-511,-198,-2844,1084,-228,-535,535,228,-1084,2844,198,511,875,1139,1194,1052,832,665,590,540,409,155,},
    {1064,1036,995,969,981,1038,1111,1138,1060,852,555,259,5252,-2519,-673,3266,3266,-673,-2519,5252,259,555,852,1060,1138,1111,1038,981,969,995,1036,1064,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {-1013,-973,-902,-805,-674,-496,-271,-35,152,238,213,121,3138,-917,3,2221,2221,3,-917,3138,121,213,238,152,-35,-271,-496,-674,-805,-902,-973,-1013,155,409,540,590,665,832,1052,1194,1139,875,511,198,2844,-1084,228,535,-535,-228,1084,-2844,-198,-511,-875,-1139,-1194,-1052,-832,-665,-590,-540,-409,-155,},
    {945,847,625,269,-182,-629,-947,-1052,-937,-677,-383,-149,-1647,2473,1572,-129,-129,1572,2473,-1647,-149,-383,-677,-937,-1052,-947,-629,-182,269,625,847,945,-131,-428,-775,-1075,-1166,-951,-489,25,376,464,345,162,3578,-410,45,300,-300,-45,410,-3578,-162,-345,-464,-376,-25,489,951,1166,1075,775,428,131,},
    {-1044,-665,-49,570,959,980,654,152,-295,-513,-466,-259,-5280,4277,2764,-1786,-1786,2764,4277,-5280,-259,-466,-513,-295,152,654,980,959,570,-49,-665,-1044,316,833,1051,895,432,-157,-655,-902,-857,-608,-306,-82,1548,1915,-241,-547,547,241,-1915,-1548,82,306,608,857,902,655,157,-432,-895,-1051,-833,-316,},
    {1026,375,-491,-1012,-888,-259,439,807,728,390,73,-59,-3821,1328,1823,-458,-458,1823,1328,-3821,-59,73,390,728,807,439,-259,-888,-1012,-491,375,1026,-484,-1126,-1022,-274,589,1009,781,145,-445,-667,-517,-223,-1786,3210,599,-528,528,-599,-3210,1786,223,517,667,445,-145,-781,-1009,-589,274,1022,1126,484,},
    {-499,-104,371,553,304,-171,-507,-468,-121,251,391,264,4425,-7033,-3026,5368,5368,-3026,-7033,4425,264,391,251,-121,-468,-507,-171,304,553,371,-104,-499,249,553,427,-29,-464,-552,-237,248,577,580,340,81,-3386,-791,4189,2767,-2767,-4189,791,3386,-81,-340,-580,-577,-248,237,552,464,29,-427,-553,-249,},
};

#endif

/*****************************************************************************
  2 ??????
*****************************************************************************/
/*****************************************************************************
  3 ????
*****************************************************************************/
oal_void hmac_add_bound(oal_uint32 *pul_number, oal_uint32 ul_bound)
{
    *pul_number = *pul_number + 1;

    if (*pul_number > (ul_bound - 1))
    {
        *pul_number -= ul_bound;
    }
}

/*lint -e571*/
/*lint -e801*/


oal_uint32 hmac_send_cali_matrix_data(mac_vap_stru *pst_mac_vap)
{

    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    dmac_tx_event_stru       *pst_h2d_matrix_data_event;
    oal_netbuf_stru          *pst_netbuf_matrix_data;
    oal_uint8                *puc_matrix_data_dst;
    oal_uint8                *puc_matrix_addr;
    oal_uint8                *puc_param;
    oal_uint32                ul_ret;
    oal_uint16                us_frame_len;
    oal_int32                 l_remain_len;
    oal_uint8                 uc_matrix_idx;
    oal_uint16               *pus_matrix_data_addr[HI1102_CALI_MATRIX_DATA_NUMS]
                                              = {g_as_rxiq_h_comp_80M[0],
                                                 g_as_rxiq_h_comp_160M[0],
                                            #if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
                                                 g_as_txiq_h_comp_320M[0],
                                            #else
                                                 g_as_txiq_h_comp_160M[0],
                                            #endif
                                            };
    oal_uint32                aul_matrix_data_size[HI1102_CALI_MATRIX_DATA_NUMS]
                                              = {
                                                 704,   /*  大小:16bit * 11 * 32 = 704 Byte */
                                                 1408,  /*  大小:16bit * 11 * 64 = 1408 Byte */
                                            #if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
                                                 2816,  /*  大小:16bit * 11 * 128 = 2816 Byte */
                                            #else
                                                 1408,  /*  大小:16bit * 11 * 64 = 1408 Byte */
                                            #endif
                                                 };

    pst_netbuf_matrix_data = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);

    if (OAL_PTR_NULL == pst_netbuf_matrix_data)
    {
       OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_send_cali_matrix_data::pst_netbuf alloc null SIZE[%d].}", WLAN_LARGE_NETBUF_SIZE);
       return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (uc_matrix_idx = 0; uc_matrix_idx < HI1102_CALI_MATRIX_DATA_NUMS; uc_matrix_idx++)
    {
        l_remain_len    = aul_matrix_data_size[uc_matrix_idx];
        puc_matrix_addr = (oal_uint8 *)pus_matrix_data_addr[uc_matrix_idx];

        puc_param     = (oal_uint8 *)(puc_matrix_addr) + l_remain_len;

        while (l_remain_len > 0)
        {
            pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
            if (OAL_PTR_NULL == pst_event_mem)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_send_cali_matrix_data::pst_event_mem null.}");
                oal_netbuf_free(pst_netbuf_matrix_data);
                return OAL_ERR_CODE_PTR_NULL;
            }

            pst_event = frw_get_event_stru(pst_event_mem);
            FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                               FRW_EVENT_TYPE_WLAN_CTX,
                               DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_MATRIX_HMAC2DMAC,
                               OAL_SIZEOF(dmac_tx_event_stru),
                               FRW_EVENT_PIPELINE_STAGE_1,
                               pst_mac_vap->uc_chip_id,
                               pst_mac_vap->uc_device_id,
                               pst_mac_vap->uc_vap_id);

            OAL_MEMZERO(oal_netbuf_cb(pst_netbuf_matrix_data), OAL_TX_CB_LEN);
            puc_matrix_data_dst = (oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_matrix_data));

            /* netbuf len 不能超过1544字节 */
            us_frame_len = (oal_uint16)OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len);
            /*lint -e662*/
            oal_memcopy(puc_matrix_data_dst, puc_param - l_remain_len, (oal_uint32)us_frame_len);
            /*lint +e662*/
            pst_h2d_matrix_data_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
            pst_h2d_matrix_data_event->pst_netbuf   = pst_netbuf_matrix_data;
            pst_h2d_matrix_data_event->us_frame_len = us_frame_len;
            pst_h2d_matrix_data_event->us_remain    = (oal_uint16)l_remain_len;

            l_remain_len -= (WLAN_LARGE_NETBUF_SIZE - 104);

            ul_ret = frw_event_dispatch_event(pst_event_mem);
            if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
            {
                OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE,
                            "{hmac_send_cali_matrix_data::dispatch event fail ret[%d],l_remain_len[%d]frame_len[%d].}",
                            ul_ret, l_remain_len, us_frame_len);

                oal_netbuf_free(pst_netbuf_matrix_data);
                FRW_EVENT_FREE(pst_event_mem);
                return OAL_FAIL;
            }

            FRW_EVENT_FREE(pst_event_mem);
        }
    }

    oal_netbuf_free(pst_netbuf_matrix_data);
    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
OAL_STATIC oal_int32 hmac_print_cail_result(oal_uint8 uc_cali_chn_idx,
                                                  oal_int8 *pc_print_buff,
                                                  oal_int32 l_remainder_len,
                                                  oal_cali_param_stru *pst_cali_data)
{
    oal_int8 *pc_string;
    if (l_remainder_len <= 0)
    {
        OAM_ERROR_LOG3(0, OAM_SF_CALIBRATE, "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
                        l_remainder_len, OAM_REPORT_MAX_STRING_LEN, OAL_SIZEOF(oal_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = (uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM)

         ? "5G 20M: cali data ch_index[%u]\n"
                "ppf[0x%x]"
             #ifdef _PRE_WLAN_NEW_RXDC
                "rx_dc_comp analog0[0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
             #else
                "rx_dc_comp analog[0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
             #endif
                "tx_power:ppa[0x%x] upc_mx_cmp[0x%x] atx_pwr[0x%x] dtx_pwr[0x%x]\n"
             #ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
                "dyn tx_power[tx_power_dc_q:0x%x]\n"
             #endif
             #ifdef _PRE_WLAN_NEW_TXDC
                "tx_dc_i_q[0x%x 0x%x]\n"
              #else
                "tx_dc_i_q[0x%x 0x%x]\n"
             #endif
         : "5G 80M: cali data index[%u]\n"
                "ppf[0x%x]"
            #ifdef _PRE_WLAN_NEW_RXDC
                "rx_dc_comp analog0[0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
            #else
                "rx_dc_comp analog[0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
            #endif
                "tx_power:ppa[0x%x] upc_mx_cmp[0x%x] atx_pwr[0x%x] dtx_pwr[0x%x]\n"
            #ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
                "dyn tx_power[tx_power_dc_q:0x%x]\n"
            #endif
            #ifdef _PRE_WLAN_NEW_TXDC
                "tx_dc_i_q[0x%x 0x%x]\n"
            #else
                "tx_dc_i_q[0x%x 0x%x]\n"
            #endif
               ;

    return OAL_SPRINTF(pc_print_buff, (oal_uint32)l_remainder_len, pc_string,
        uc_cali_chn_idx,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_ppf_cmp_val.uc_ppf_val,
    #ifdef _PRE_WLAN_NEW_RXDC
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.aus_analog_rxdc_cmp[0],
    #else
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_analog_rxdc_cmp,
    #endif
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_digital_rxdc_cmp_i,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_digital_rxdc_cmp_q,

        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.upc_ppa_cmp,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.upc_mx_cmp,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.ac_atx_pwr_cmp,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.dtx_pwr_cmp,
    #ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.s_5g_tx_power_dc_q,
    #endif

    #ifdef _PRE_WLAN_NEW_TXDC
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val[0].us_txdc_cmp_i,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val[0].us_txdc_cmp_q
    #else
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_i,
        pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_q
    #endif
    );

}

#else
OAL_STATIC oal_int32 hmac_print_cail_result(oal_uint8 uc_cali_chn_idx,
                                                  oal_int8 *pc_print_buff,
                                                  oal_uint32 ul_remainder_len,
                                                  oal_cali_param_stru *pst_cali_data)
{
    oal_int8 *pc_string;

    pc_string = (uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M cali data index:%d, rx_dc_comp:0x%x, s_digital_rxdc_cmp_i:0x%x, us_digital_rxdc_cmp_q:0x%x \n"
          "5G 20M tx_power upc_ppa_cmp:0x%x, upc_mx_cmp:0x%x, atx_pwr_cmp:0x%x,  ppf:0x%x \n"
          "5G 20M tx_dc i:%u,  q:%u, tx_iq p:%u  e:%u \n"
        : "5G 80M cali data index:%d, rx_dc_comp:0x%x, s_digital_rxdc_cmp_i:0x%x, us_digital_rxdc_cmp_q:0x%x \n"
          "5G 80M tx_power upc_ppa_cmp:0x%x, upc_mx_cmp:0x%x, atx_pwr_cmp:0x%x,  ppf:0x%x \n"
          "5G 80M tx_dc i:%u,  q:%u, tx_iq p:%u  e:%u \n" ;
    return OAL_SPRINTF(pc_print_buff, ul_remainder_len, pc_string,
                    uc_cali_chn_idx,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_analog_rxdc_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_digital_rxdc_cmp_i,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_5G.us_digital_rxdc_cmp_q,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.upc_ppa_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.upc_mx_cmp,
                    (oal_uint8)pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_5G.ac_atx_pwr_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_ppf_cmp_val.uc_ppf_val,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_i,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_q,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txiq_cmp_val_5G.us_txiq_cmp_p,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].g_st_txiq_cmp_val_5G.us_txiq_cmp_e);

}
#endif

extern oal_uint32 band_5g_enabled;
oal_void hmac_dump_cali_result(oal_void)
{
    oal_cali_param_stru *pst_cali_data;
    oal_uint8            uc_cali_chn_idx;
    oal_uint32           ul_string_len;
    oal_int32            l_string_tmp_len;
    oal_int8            *pc_print_buff;
    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "rc code RC:0x%x, R:0x%x, C:0x%x, check_hw_status:0x%x",
            pst_cali_data->st_bfgn_cali_data.g_uc_rc_cmp_code,
            pst_cali_data->st_bfgn_cali_data.g_uc_r_cmp_code,
            pst_cali_data->st_bfgn_cali_data.g_uc_c_cmp_code,
            pst_cali_data->ul_check_hw_status);
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        OAM_WARNING_LOG4(0, OAM_SF_CFG, "2G cali data index:%u, rx_dc_lna1_comp: 0x%x, tx_dc txdc_cmp_i:0x%x txdc_cmp_q:0x%x",
                uc_cali_chn_idx,
            #ifdef _PRE_WLAN_NEW_RXDC
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_2G.aus_analog_rxdc_cmp[1],
            #else
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_cali_rx_dc_cmp_2G.us_analog_rxdc_cmp,
            #endif
            #ifdef _PRE_WLAN_NEW_TXDC
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val[0].us_txdc_cmp_i,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val[0].us_txdc_cmp_q
            #else
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_i,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txdc_cmp_val.us_txdc_cmp_q
            #endif
            );
        OAM_WARNING_LOG4(0, OAM_SF_CFG, "2G cali data index:%u, upc_ppa_cmp:0x%x, ac_atx_pwr_cmp:0x%x, dtx_pwr_cmp:0x%x",
                uc_cali_chn_idx,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_2G.upc_ppa_cmp,
                (oal_uint8)pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_2G.ac_atx_pwr_cmp,
                (oal_uint8)pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_cali_tx_power_cmp_2G.dtx_pwr_cmp);

        OAM_WARNING_LOG4(0, OAM_SF_CFG, " 2G tx_iq txiq_cmp_alpha:0x%x, txiq_cmp_beta:0x%x, rx_iq txiq_cmp_alpha:0x%x, txiq_cmp_beta:0x%x",
            #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST) && defined (_PRE_WLAN_NEW_IQ)
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.l_txiq_cmp_alpha,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.l_txiq_cmp_beta,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_alpha,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_beta
            #else
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txiq_cmp_val_2G.us_txiq_cmp_p,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].g_st_txiq_cmp_val_2G.us_txiq_cmp_e,
                0,
                0
            #endif
                );

    }

    if (OAL_FALSE == band_5g_enabled)
    {
        return;
    }

    pc_print_buff = (oal_int8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (OAL_PTR_NULL == pc_print_buff)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_dump_cali_result::pc_print_buff null.}", OAM_REPORT_MAX_STRING_LEN);
        return;
    }

    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;

    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = hmac_print_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;

    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    /* ???????OAM_REPORT_MAX_STRING_LEN,???oam_print */
    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;

    for (uc_cali_chn_idx = OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx < OAL_5G_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = hmac_print_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;

    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    /* ???????OAM_REPORT_MAX_STRING_LEN,???oam_print */
    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;

#ifdef _PRE_WLAN_NEW_IQ
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "RX_IQ udelay1:0x%x, udelay2:0x%x, alpha:0x%x, beta:0x%x \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_u1,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_u2,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_alpha,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_beta);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    /* ???????OAM_REPORT_MAX_STRING_LEN,???oam_print */
    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "TX_IQ alpha0:0x%x, alpha:0x%x, beta0:0x%x, beta:0x%x \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_alpha0_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_alpha_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_beta0_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_beta_reg_val[7]);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
#else
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "RX_IQ rxiq_cmp alpha:0x%x,rxiq_cmp beta:0x%x,udelay1:0x%x,udelay2:0x%x, \n"
            "RX_IQ rxiq_comp_ls_filt0~10[%d %d %d %d %d %d %d %d %d %d %d] \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_alpha,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_beta,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_u1,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.l_rxiq_cmp_u2,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[0],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[1],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[2],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[3],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[4],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[5],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[6],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[7],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[8],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[9],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_rxiq_cmp_val.as_rxiq_comp_ls_filt[10]);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    /* ???????OAM_REPORT_MAX_STRING_LEN,???oam_print */
    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "TX_IQ_time_cmp_val alpha:0x%x, beta:0x%x \n"
            "TX_IQ txiq_comp_ls_filt0~10[%d %d %d %d %d %d %d %d %d %d %d] \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.l_txiq_cmp_alpha,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.l_txiq_cmp_beta,
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[0],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[1],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[2],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[3],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[4],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[5],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[6],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[7],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[8],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[9],
            pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_new_txiq_time_cmp_val.as_txiq_comp_ls_filt[10]);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
#endif
#endif

    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);
    return;

sprint_fail:

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);

    return;

}
/*lint +e801*/
/*lint +e571*/

oal_uint32  hmac_save_cali_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    hal_cali_hal2hmac_event_stru    *pst_cali_save_event;
    hal_cali_hal2hmac_payload_stru *pst_pay_load;
    oal_uint8 *puc_start_addr;
    oal_uint32 ul_copy_len;
    oal_update_cali_channel_stru *pst_update_cali_channel;
    oal_cali_param_stru *pst_cali_data;
    oal_uint32                      ul_remain_len;
    oal_uint32                      ul_netbuf_len;
    //oal_uint8  *puc_content;
    //oal_uint32 ul_byte;

    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_cali_save_event = (hal_cali_hal2hmac_event_stru *)pst_event->auc_event_data;

    pst_pay_load  = (hal_cali_hal2hmac_payload_stru *)OAL_NETBUF_DATA(pst_cali_save_event->pst_netbuf);
    pst_update_cali_channel = &pst_cali_data->st_cali_update_info;
    //pst_update_cali_channel->ul_cali_time = 0;


    //OAL_IO_PRINT("pst_update_cali_channel->ul_cali_time %d : \r\n", pst_update_cali_channel->ul_cali_time);
    ul_netbuf_len = OAL_NETBUF_LEN(pst_cali_save_event->pst_netbuf);

    //OAL_IO_PRINT("hmac_save_cali_event : first cali packet idx:pst_pay_load->ul_packet_idx %d\r\n", pst_pay_load->ul_packet_idx);
    OAM_WARNING_LOG1(0, 0, "hmac_save_cali_event : first cali packet idx:pst_pay_load->ul_packet_idx %d\r\n", pst_pay_load->ul_packet_idx);

    puc_start_addr = ((oal_uint8 *)pst_cali_data->ast_2Gcali_param) + ((ul_netbuf_len - 4) * pst_pay_load->ul_packet_idx);
    ul_remain_len = OAL_WIFI_CALI_DATA_UPLOAD_LEN;
    ul_remain_len = ul_remain_len - (ul_netbuf_len - 4) * pst_pay_load->ul_packet_idx;
    //OAL_IO_PRINT("hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);

    //OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);
    //OAL_IO_PRINT("hmac_save_cali_event : WLAN_LARGE_NETBUF_SIZE %d\r\n", ul_netbuf_len);

    if (ul_remain_len <= ul_netbuf_len - 4)
    {
        //OAL_IO_PRINT("hmac_save_cali_event : last packet");
        ul_copy_len = ul_remain_len;

        if ((pst_update_cali_channel->ul_cali_time == 0) && (g_uc_netdev_is_open == OAL_FALSE))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_save_cali_event: wlan_pm_close,g_uc_netdev_is_open = FALSE!\r\n.}");
            wlan_pm_close();
        }
    }
    else
    {
        ul_copy_len = ul_netbuf_len - 4;
    }

    oal_memcopy(puc_start_addr, pst_pay_load->auc_payoald, ul_copy_len);

    if (ul_remain_len <= ul_netbuf_len - 4)
    {
        pst_update_cali_channel->ul_cali_time++;
        pst_update_cali_channel->en_update_bt = OAL_FALSE;
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_save_cali_event: : last packet ul_cali_time %d}", pst_update_cali_channel->ul_cali_time);
        hmac_dump_cali_result();
    }

#if 0

    puc_content = (oal_uint8 *)pst_cali_data->ast_2Gcali_param;

    OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : calidata len%d \r\n", OAL_SIZEOF(oal_cali_param_stru) - 8);
    for (ul_byte = 0; ul_byte < OAL_SIZEOF(oal_cali_param_stru) - 8; ul_byte+=4)
    {

        OAL_IO_PRINT("%02X %02X %02X %02X\r\n", puc_content[ul_byte], puc_content[ul_byte+1],
                      puc_content[ul_byte+2], puc_content[ul_byte+3]);
    }

    else
    {
        puc_start_addr = pst_pay_load->auc_payoald;
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1 + HAL_5G_20M_CHANNEL_NUM],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        if (pst_update_cali_channel->en_update_bt == OAL_TRUE)
        {
            puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
            oal_memcopy(&g_st_cali_control.st_bt_cali_comp,
                        pst_pay_load,
                        OAL_SIZEOF(oal_bt_cali_comp_stru));
        }

        pst_update_cali_channel->ul_cali_time++;
        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_2g_band_idx, HAL_2G_CHANNEL_NUM + 1);

        if (HAL_2G_CHANNEL_NUM == pst_update_cali_channel->uc_2g_band_idx)
        {
            pst_update_cali_channel->en_update_bt = OAL_TRUE;
        }
        else
        {
            pst_update_cali_channel->en_update_bt = OAL_FALSE;
        }

        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);

        while (mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, pst_update_cali_channel->uc_5g_chan_idx1))
        {
            hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);
        }
    }

#endif

    oal_netbuf_free(pst_cali_save_event->pst_netbuf);

    return OAL_SUCC;
}

oal_uint32 hmac_send_cali_data(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    dmac_tx_event_stru       *pst_dpd_event;
    oal_netbuf_stru          *pst_netbuf_cali_data;
    oal_uint8                *puc_cali_data;
    oal_cali_param_stru      *pst_cali_data;
    oal_int16                 l_remain_len;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CALIBRATE, "{hmac_dpd_data_processed_send::param null, %p %p.}", pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }



    l_remain_len = OAL_WIFI_CALI_DATA_DOWNLOAD_LEN;
    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();
    hmac_dump_cali_result();

    while (l_remain_len > 0)
    {
        pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
        if (OAL_PTR_NULL == pst_event_mem)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_scan_proc_scan_req_event::pst_event_mem null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }

        OAL_IO_PRINT("{hmac_send_cali_data.start}\r\n");

        pst_event = (frw_event_stru *)pst_event_mem->puc_data;

        FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                           FRW_EVENT_TYPE_WLAN_CTX,
                           DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC,
                           OAL_SIZEOF(dmac_tx_event_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_mac_vap->uc_chip_id,
                           pst_mac_vap->uc_device_id,
                           pst_mac_vap->uc_vap_id);

        pst_netbuf_cali_data = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
        if (OAL_PTR_NULL == pst_netbuf_cali_data)
        {
           FRW_EVENT_FREE(pst_event_mem);
           OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE,"{hmac_dpd_data_processed_send::pst_netbuf_scan_result null.}");

           return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        OAL_MEMZERO(oal_netbuf_cb(pst_netbuf_cali_data), OAL_TX_CB_LEN);


        /*lint -e416*/
        /* ???????????????? */
        puc_param = (oal_uint8 *)(pst_cali_data->ast_2Gcali_param) + OAL_WIFI_CALI_DATA_DOWNLOAD_LEN - l_remain_len;

        puc_cali_data = (oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_cali_data));
        /*lint -e571*/
        /* ??????????buf????,??????,????????,????device????? */
        oal_memcopy(puc_cali_data, puc_param, (oal_uint32)(OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len)));

        pst_dpd_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
        pst_dpd_event->pst_netbuf   = pst_netbuf_cali_data;
        pst_dpd_event->us_frame_len = (oal_uint16)(OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len));
        pst_dpd_event->us_remain    = (oal_uint16)l_remain_len;

        l_remain_len -= (WLAN_LARGE_NETBUF_SIZE - 104);

        frw_event_dispatch_event(pst_event_mem);

        oal_netbuf_free(pst_netbuf_cali_data);
        FRW_EVENT_FREE(pst_event_mem);

    }


    return OAL_SUCC;
}

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



