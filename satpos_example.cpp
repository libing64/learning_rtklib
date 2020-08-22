/*--------------------------
satpos estimation
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


/* satpos estimation*/
int main(int argc, char**argv)
{
    char file[] = "../data/rinex/daej229a0*.20n";//2020.08.16
    nav_t nav = {0};
    double ep[] = {2020, 8, 16, 0, 0, 0};
    double rs[6] = {0}, dts[2] = {0};
    double var;
    gtime_t t, time;
    int svh = 0;

    time = epoch2time(ep);
    readrnx(file, 1, "", NULL, &nav, NULL);

    traceopen("satpos.trace");
    tracelevel(3);

    for (int i = 0; i < 3600; i += 30)
    {
        t = timeadd(time, (double)i);
        double epoch[6] = {0};
        time2epoch(t, epoch);
        printf("%lf,%lf,%lf,%lf,%lf,%lf\n", epoch[0], epoch[1], epoch[2], epoch[3], epoch[4], epoch[5]);

        for (int sat = 0; sat < MAXPRNGPS; sat++)
        {
            int ret = satpos(t, t, sat, EPHOPT_BRDC, &nav, rs, dts, &var, &svh);
            if (ret)//1:OK; 0:error
            {
                printf("%02d %6d %14.3f %14.3f %14.3f %14.3f\n",
                       sat, i, rs[0], rs[1], rs[2], dts[0] * 1E9);
            }
        }

    }
    traceclose();
    return 0;
}

