#include <stdio.h>
#include <assert.h>
#include <rtklib.h>
#include <iostream>
using namespace std;

static int nextobsf(const obs_t *obs, int *i)
{
    double tt;
    int n;

    for (n = 0; *i + n < obs->n; n++)
    {
        tt = timediff(obs->data[*i + n].time, obs->data[*i].time);
        if (tt > DTTOL)
            break;
    }
    return n;
}

int main(int argc, char **argv)
{
    gtime_t t0 = {0}, ts = {0}, te = {0};
    char file1[] = "../data/sp3/igr2119*.sp3"; /* 2020/8/16 */
    char file2[] = "../data/sp3/igr2119*.clk"; /* 2020/8/16 */
    char file3[] = "../data/rinex/daej229a00.20n";/*2020.08.16*/
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


    traceopen("ppp.trace");
    tracelevel(3);

    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;

    prcopt.mode = PMODE_PPP_STATIC;
    prcopt.navsys = SYS_GPS;
    prcopt.sateph = EPHOPT_PREC;
    prcopt.refpos = 1;
    prcopt.glomodear = 0;
    solopt.timef = 0;
    solopt.posf = SOLF_LLH;
    sprintf(solopt.prog, "ver.%s", VER_RTKLIB);

    rtk_t rtk;
    rtkinit(&rtk, &prcopt);

    for (int i = 0; i < NSATGPS; i++)
    {
        nav.lam[i][0] = CLIGHT / FREQ1;
        nav.lam[i][1] = CLIGHT / FREQ2;
        nav.lam[i][2] = CLIGHT / FREQ5;
    }


    int m = 0;
    for (int i = 0; (m = nextobsf(&obs, &i)) > 0; i += m)
    {
        rtk.sol.time = obs.data[i].time;
        rtkpos(&rtk, &obs.data[i], m, &nav);

        sol_t *sol = &rtk.sol;
        if (sol->stat == SOLQ_PPP)
        {
            printf("%d/%d, pos: %lf,%lf,%lf\n", i, obs.n, sol->rr[0], sol->rr[1], sol->rr[2]);
            //printf("sat: %d, rcv: %d\n", obs.data[i].sat, obs.data[i].rcv);
            //printf("qr: %lf,%lf,%lf,%lf,%lf,%lf\n", sol->qr[0], sol->qr[1], sol->qr[2], sol->qr[3], sol->qr[4], sol->qr[5]);
        }
        else
        {
            printf("(%d,%d)/%d, type: %d\n", i, m, obs.n, sol->type);
            printf("msg: %s\n\n", rtk.errbuf);
        }
    }

    traceclose();
    rtkfree(&rtk);
    free(obs.data);
    return 0;
}