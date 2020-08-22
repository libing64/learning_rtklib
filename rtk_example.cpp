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
    char file1[] = "../data/07590920.05n";
    char file2[] = "../data/07590920.05o";
    char file3[] = "../data/30400920.05o";
    int ret;
    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {""};

    cout << "readin rinex file: " << file2 << endl;
    cout << "================================" << endl;
    ret = readrnxt(file1, 1, ts, te, 0.0, "", &obs, &nav, &sta);

    //1 -> rover 
    ret = readrnxt(file2, 1, t0, t0, 0.0, "", &obs, &nav, &sta);


    cout << "LINE: " << __LINE__ << endl;
    //2 -> base station
    ret = readrnxt(file3, 2, t0, t0, 0.0, "", &obs, &nav, &sta);
    sortobs(&obs);

    dumpobs(&obs);

    printf("station pos: %lf,%lf,%lf\n", sta.pos[0], sta.pos[1], sta.pos[2]);
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;

    prcopt.mode = PMODE_KINEMA;
    prcopt.nf = 2;
    prcopt.modear = 2;
    prcopt.navsys = SYS_GPS;
    prcopt.refpos = 0;
    prcopt.rb[0] = sta.pos[0];
    prcopt.rb[1] = sta.pos[1];
    prcopt.rb[2] = sta.pos[2];
    solopt.posf = SOLF_ENU;
    sprintf(solopt.prog, "ver.%s", VER_RTKLIB);

    rtk_t rtk;
    rtkinit(&rtk, &prcopt);

    for (int i = 0; i < NSATGPS; i++)
    {
        nav.lam[i][0] = CLIGHT / FREQ1;
        nav.lam[i][1] = CLIGHT / FREQ2;
        nav.lam[i][2] = CLIGHT / FREQ5;
    }


    traceopen("rtk.trace");
    tracelevel(0);
    int m = 0;
    int rcv = 1;
    for (int i = 0; (m = nextobsf(&obs, &i)) > 0; i += m)
    {
        rtk.sol.time = obs.data[i].time;
        int stat = rtkpos(&rtk, &obs.data[i], m, &nav);
        cout << "stat: " << stat << endl;
        sol_t *sol = &rtk.sol;

        if (stat == 1)
        {    
            printf("(%d,%d)/%d, type: %d, pos: %lf,%lf,%lf\n", i, m, obs.n, sol->type, sol->rr[0] - rtk.rb[0], sol->rr[1] - rtk.rb[1], sol->rr[2] - rtk.rb[2]);
            printf("qr: %lf,%lf,%lf,%lf,%lf,%lf\n", sol->qr[0], sol->qr[1], sol->qr[2], sol->qr[3], sol->qr[4], sol->qr[5]);
            printf("solution status: %u, valid sat: %d\n", sol->stat, sol->ns);

            unsigned char buff[4096];
            double rb[3];

            outprcopts(buff, &prcopt);
            printf("opt: %s\n", buff);



            outsolheads(buff, &solopt);
            printf("sol head: %s\n", buff);


            outsols(buff, sol, rb, &solopt);
            printf("sols: %s\n", buff);
            printf("rb: %lf, %lf, %lf\n", rb[0], rb[1], rb[2]);
            printf("msg: %s\n\n", rtk.errbuf);

 
        }
        else
        {
            
            printf("(%d,%d)/%d, type: %d\n", i, m, obs.n, sol->type);
            printf("msg: %s\n\n", rtk.errbuf);
        }
    }



    traceclose();
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