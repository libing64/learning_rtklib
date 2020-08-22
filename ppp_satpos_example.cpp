/*--------------------------
precise ephemeris parser
*---------------------------*/
#include <stdio.h>
#include <assert.h>
#include <rtklib.h>
#include <iostream>
using namespace std;

static void dumpeph(peph_t *peph, int n)
{
    char s[64];
    int i, j;
    for (i = 0; i < n; i++)
    {
        time2str(peph[i].time, s, 3);
        printf("time=%s\n", s);
        for (j = 0; j < MAXSAT; j++)
        {
            printf("%03d: %14.3f %14.3f %14.3f : %5.3f %5.3f %5.3f\n",
                   j + 1, peph[i].pos[j][0], peph[i].pos[j][1], peph[i].pos[j][2],
                   peph[i].std[j][0], peph[i].std[j][1], peph[i].std[j][2]);
        }
    }
}
static void dumpclk(pclk_t *pclk, int n)
{
    char s[64];
    int i, j;
    for (i = 0; i < n; i++)
    {
        time2str(pclk[i].time, s, 3);
        printf("time=%s\n", s);
        for (j = 0; j < MAXSAT; j++)
        {
            printf("%03d: %14.3f : %5.3f\n",
                   j + 1, pclk[i].clk[j][0] * 1E9, pclk[i].std[j][0] * 1E9);
        }
    }
}


/* peph2pos() */
int main(int argc, char**argv)
{
    char file1[] = "../data/sp3/igr2119*.sp3"; /* 2020/8/16 */
    char file2[] = "../data/sp3/igr2119*.clk"; /* 2020/8/16 */
    nav_t nav = {0};
    int stat, sat;
    double ep[] = {2020, 8, 16, 0, 0, 0};
    double rs[6] = {0}, dts[2] = {0};
    double var;
    gtime_t t, time;

    time = epoch2time(ep);

    readsp3(file1, &nav, 0);
    dumpeph(nav.peph, nav.ne);

    readrnxc(file2, &nav);
    dumpclk(nav.pclk, nav.nc);

    sat = 4;
    for (int i = 0; i < 86400 * 2; i += 30)
    {
        t = timeadd(time, (double)i);
        for (int j = 0; j < 6; j++)
            rs[j] = 0.0;
        for (int j = 0; j < 2; j++)
            dts[j] = 0.0;
        peph2pos(t, sat, &nav, 0, rs, dts, &var);
        printf("%02d %6d %14.3f %14.3f %14.3f %14.3f %10.3f %10.3f %10.3f %10.3f\n",
                sat, i, rs[0], rs[1], rs[2], dts[0] * 1E9, rs[3], rs[4], rs[5], dts[1] * 1E9);
    }

    return 0;
}

