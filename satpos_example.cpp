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
    char *file1 = "../data/sp3/igs1590*.sp3"; /* 2010/7/1 */
    char *file2 = "../data/sp3/igs1590*.clk"; /* 2010/7/1 */
    char *file3 = "../../data/igs05.atx";
    char *file4 = "../data/rinex/brdc*.10n";
    pcvs_t pcvs = {0};
    pcv_t *pcv;
    nav_t nav = {0};
    int i, stat, sat, svh;
    double ep[] = {2010, 7, 1, 0, 0, 0};
    double rs1[6] = {0}, dts1[2] = {0}, rs2[6] = {0}, dts2[2] = {0};
    double var;
    gtime_t t, time;

    time = epoch2time(ep);

    readsp3(file1, &nav, 0);
    readrnxc(file2, &nav);
    stat = readpcv(file3, &pcvs);
    readrnx(file4, 1, "", NULL, &nav, NULL);

    for (i = 0; i < MAXSAT; i++)
    {
        if (!(pcv = searchpcv(i + 1, "", time, &pcvs)))
            continue;
        nav.pcvs[i] = *pcv;
    }

    sat = 3;

    for (i = 0; i < 86400 * 2; i += 30)
    {
        t = timeadd(time, (double)i);
        satpos(t, t, sat, EPHOPT_BRDC, &nav, rs1, dts1, &var, &svh);
        satpos(t, t, sat, EPHOPT_PREC, &nav, rs2, dts2, &var, &svh);
        printf("%02d %6d %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f\n",
                sat, i,
                rs1[0], rs1[1], rs1[2], dts1[0] * 1E9, rs2[0], rs2[1], rs2[2], dts2[0] * 1E9);
    }
    return 0;
}

