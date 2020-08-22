#include <stdio.h>
#include <assert.h>
#include <rtklib.h>
#include <iostream>
using namespace std;


static int nextobsf(const obs_t *obs, int *i, int rcv)
{
    double tt;
    int n;

    for (; *i < obs->n; (*i)++)
        if (obs->data[*i].rcv == rcv)
            break;
    for (n = 0; *i + n < obs->n; n++)
    {
        tt = timediff(obs->data[*i + n].time, obs->data[*i].time);
        if (obs->data[*i + n].rcv != rcv || tt > DTTOL)
            break;
    }
    return n;
}

int main(int argc, char **argv)
{
    gtime_t t0 = {0}, ts = {0}, te = {0};
    char file1[] = "../data/sp3/igr2119*.sp3";     /* 2020/8/16 */
    char file2[] = "../data/sp3/igr2119*.clk";     /* 2020/8/16 */
    char file3[] = "../data/rinex/daej229a00.20n"; /*2020.08.16*/
    char file4[] = "../data/rinex/daej229a00.20o"; /*2020.08.16*/

    int ret;
    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {""};

    cout << "readin rinex file: " << file2 << endl;
    cout << "================================" << endl;
    readsp3(file1, &nav, 0);
    readrnxc(file2, &nav);

    //1 -> rover
    ret = readrnxt(file3, 1, ts, te, 0.0, "", &obs, &nav, &sta);
    ret = readrnxt(file4, 1, t0, t0, 0.0, "", &obs, &nav, &sta);

    int m = 0;
    int rcv = 1;
    double rs1[6] = {0}, dts1[2] = {0}, rs2[6] = {0}, dts2[2] = {0};
    double var;
    int svh;
    for (int i = 0; (m = nextobsf(&obs, &i, rcv)) > 0; i += m)
    {
        gtime_t t = obs.data[i].time;
        double ep[6] = {0};
        time2epoch(t, ep);
        printf("%lf,%lf,%lf,%lf,%lf,%lf\n", ep[0], ep[1], ep[2], ep[3], ep[4], ep[5]);
        for (int sat = 0; sat < 30; sat++)
        {
            int ret1 = satpos(t, t, sat, EPHOPT_BRDC, &nav, rs1, dts1, &var, &svh);
            int ret2 = satpos(t, t, sat, EPHOPT_PREC, &nav, rs2, dts2, &var, &svh);
            if (ret1 && ret2)
            {
                printf("%02d %6d %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f\n",
                       sat, i,
                       rs1[0], rs1[1], rs1[2], dts1[0] * 1E9, rs2[0], rs2[1], rs2[2], dts2[0] * 1E9);
            }
        }
    }
    return 0;
}