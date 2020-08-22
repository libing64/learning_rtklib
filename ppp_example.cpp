#include <stdio.h>
#include <assert.h>
#include <rtklib.h>
#include <iostream>
using namespace std;


static void dumpobs(obs_t *obs)
{
    gtime_t time = {0};
    int i;
    char str[64];
    printf("obs : n=%d\n", obs->n);
    for (i = 0; i < obs->n; i++)
    {
        time2str(obs->data[i].time, str, 3);
        printf("%s : %2d %2d %13.3f %13.3f %13.3f %13.3f  %d %d\n", str, obs->data[i].sat,
               obs->data[i].rcv, obs->data[i].L[0], obs->data[i].L[1],
               obs->data[i].P[0], obs->data[i].P[1], obs->data[i].LLI[0], obs->data[i].LLI[1]);
        time = obs->data[i].time;
    }
}
static void dumpnav(nav_t *nav)
{
    int i;
    char str[64], s1[64], s2[64];
    printf("nav : n=%d\n", nav->n);
    for (i = 0; i < nav->n; i++)
    {
        time2str(nav->eph[i].toe, str, 3);
        time2str(nav->eph[i].toc, s1, 0);
        time2str(nav->eph[i].ttr, s2, 0);
        printf("%s : %2d    %s %s %3d %3d %2d\n", str, nav->eph[i].sat, s1, s2,
               nav->eph[i].iode, nav->eph[i].iodc, nav->eph[i].svh);

        assert(nav->eph[i].iode == (nav->eph[i].iodc & 0xFF));
    }
}
static void dumpsta(sta_t *sta)
{
    printf("name    = %s\n", sta->name);
    printf("marker  = %s\n", sta->marker);
    printf("antdes  = %s\n", sta->antdes);
    printf("antsno  = %s\n", sta->antsno);
    printf("rectype = %s\n", sta->rectype);
    printf("recver  = %s\n", sta->recver);
    printf("recsno  = %s\n", sta->recsno);
    printf("antsetup= %d\n", sta->antsetup);
    printf("itrf    = %d\n", sta->itrf);
    printf("deltype = %d\n", sta->deltype);
    printf("pos     = %.3f %.3f %.3f\n", sta->pos[0], sta->pos[1], sta->pos[2]);
    printf("del     = %.3f %.3f %.3f\n", sta->del[0], sta->del[1], sta->del[2]);
    printf("hgt     = %.3f\n", sta->hgt);
}

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

int main(int argc, char **argv)
{
    gtime_t t0 = {0}, ts = {0}, te = {0};
    char file1[] = "../data/sp3/igr2119*.sp3"; /* 2020/8/16 */
    char file2[] = "../data/sp3/igr2119*.clk"; /* 2020/8/16 */
    char file3[] = "../data/renix/daej229a00.20n";/*2020.08.16*/
    char file4[] = "../data/renix/daej229a00.20o"; /*2020.08.16*/

    int ret;
    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {""};

    cout << "readin rinex file: " << file2 << endl;
    cout << "================================" << endl;
    readsp3(file1, &nav, 0);
    readrnxc(file2, &nav);

    // cout << "dump eph and ne: " << endl;
    // dumpeph(nav.peph, nav.ne);


    // cout << "dump nav: " << endl;
    // dumpnav(&nav);
    //1 -> rover
    ret = readrnxt(file3, 1, ts, te, 0.0, "", &obs, &nav, &sta);
    ret = readrnxt(file4, 1, t0, t0, 0.0, "", &obs, &nav, &sta);
    // cout << "dump obs: " << endl;
    // dumpobs(&obs);

    //sat pos estimation
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
    //return 0;

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


    FILE* pf = fopen("ppp.log", "wb");


    //int m = 0;
    //int rcv = 1;
    for (int i = 0; (m = nextobsf(&obs, &i, rcv)) > 0; i += m)
    {
        rtk.sol.time = obs.data[i].time;
        rtkpos(&rtk, &obs.data[i], m, &nav);
        pppoutsolstat(&rtk, 5, pf);


        sol_t *sol = &rtk.sol;
        printf("solution status: %hhu\n", sol->stat);
        if (sol->stat != SOLQ_NONE)
        {
            printf("%d/%d, pos: %lf,%lf,%lf\n", i, obs.n, sol->rr[0], sol->rr[1], sol->rr[2]);
            printf("sat: %d, rcv: %d\n", obs.data[i].sat, obs.data[i].rcv);
            printf("qr: %lf,%lf,%lf,%lf,%lf,%lf\n", sol->qr[0], sol->qr[1], sol->qr[2], sol->qr[3], sol->qr[4], sol->qr[5]);
            printf("solution status: %u, valid sat: %d\n", sol->stat, sol->ns);
        }
        else
        {
            printf("(%d,%d)/%d, type: %d\n", i, m, obs.n, sol->type);
            printf("msg: %s\n\n", rtk.errbuf);
        }
    }

fclose(pf);


    traceclose();
    rtkfree(&rtk);
    free(obs.data);
    return 0;
}

//extern int rtkpos(rtk_t *rtk, const obsd_t *obs, int n, const nav_t *nav)

    /* precise positioning ---------------------------------------------------------
* input observation data and navigation message, compute rover position by 
* precise positioning
* args   : rtk_t *rtk       IO  rtk control/result struct
*            rtk->sol       IO  solution
*                .time      O   solution time
*                .rr[]      IO  rover position/velocity
*                               (I:fixed mode,O:single mode)
*                .dtr[0]    O   receiver clock bias (s)
*                .dtr[1]    O   receiver glonass-gps time offset (s)
*                .Qr[]      O   rover position covarinace
*                .stat      O   solution status (SOLQ_???)
*                .ns        O   number of valid satellites
*                .age       O   age of differential (s)
*                .ratio     O   ratio factor for ambiguity validation
*            rtk->rb[]      IO  base station position/velocity
*                               (I:relative mode,O:moving-base mode)
*            rtk->nx        I   number of all states
*            rtk->na        I   number of integer states
*            rtk->ns        O   number of valid satellite
*            rtk->tt        O   time difference between current and previous (s)
*            rtk->x[]       IO  float states pre-filter and post-filter
*            rtk->P[]       IO  float covariance pre-filter and post-filter
*            rtk->xa[]      O   fixed states after AR
*            rtk->Pa[]      O   fixed covariance after AR
*            rtk->ssat[s]   IO  sat(s+1) status
*                .sys       O   system (SYS_???)
*                .az   [r]  O   azimuth angle   (rad) (r=0:rover,1:base)
*                .el   [r]  O   elevation angle (rad) (r=0:rover,1:base)
*                .vs   [r]  O   data valid single     (r=0:rover,1:base)
*                .resp [f]  O   freq(f+1) pseudorange residual (m)
*                .resc [f]  O   freq(f+1) carrier-phase residual (m)
*                .vsat [f]  O   freq(f+1) data vaild (0:invalid,1:valid)
*                .fix  [f]  O   freq(f+1) ambiguity flag
*                               (0:nodata,1:float,2:fix,3:hold)
*                .slip [f]  O   freq(f+1) slip flag
*                               (bit8-7:rcv1 LLI, bit6-5:rcv2 LLI,
*                                bit2:parity unknown, bit1:slip)
*                .lock [f]  IO  freq(f+1) carrier lock count
*                .outc [f]  IO  freq(f+1) carrier outage count
*                .slipc[f]  IO  freq(f+1) cycle slip count
*                .rejc [f]  IO  freq(f+1) data reject count
*                .gf        IO  geometry-free phase (L1-L2) (m)
*                .gf2       IO  geometry-free phase (L1-L5) (m)
*            rtk->nfix      IO  number of continuous fixes of ambiguity
*            rtk->neb       IO  bytes of error message buffer
*            rtk->errbuf    IO  error message buffer
*            rtk->tstr      O   time string for debug
*            rtk->opt       I   processing options
*          obsd_t *obs      I   observation data for an epoch
*                               obs[i].rcv=1:rover,2:reference
*                               sorted by receiver and satellte
*          int    n         I   number of observation data
*          nav_t  *nav      I   navigation messages
* return : status (0:no solution,1:valid solution)
* notes  : before calling function, base station position rtk->sol.rb[] should
*          be properly set for relative mode except for moving-baseline
*-----------------------------------------------------------------------------*/