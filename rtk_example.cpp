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

static int get_sat_cnt(const obs_t *obs, int i, int m, int rcv)
{
    int cnt = 0;
    for (int k = i; k < (i + m); k++)
    {
        if (obs->data[k].rcv == rcv)
        {
            cnt++;
        }
    }
    return cnt;
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

    //2 -> base station
    ret = readrnxt(file3, 2, t0, t0, 0.0, "", &obs, &nav, &sta);
    sortobs(&obs);

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
        int rover_station_cnt = get_sat_cnt(&obs, i, m, 1);
        int base_station_cnt = get_sat_cnt(&obs, i, m, 2);
        //printf("i: %d, m: %d, base_sat_cnt : %d, rover_sat_cnt: %d\n", i, m, base_station_cnt, rover_station_cnt);
        if (base_station_cnt == 0 || rover_station_cnt == 0)
        {
            continue;
        }

        rtk.sol.time = obs.data[i].time;
        int stat = rtkpos(&rtk, &obs.data[i], m, &nav);
        sol_t *sol = &rtk.sol;

        if (stat == SOLQ_FIX)//fix solution
        {    
            printf("(%d,%d)/%d, type: %d, pos: %lf,%lf,%lf\n", i, m, obs.n, sol->type, sol->rr[0] - rtk.rb[0], sol->rr[1] - rtk.rb[1], sol->rr[2] - rtk.rb[2]);
            //printf("qr: %lf,%lf,%lf,%lf,%lf,%lf\n", sol->qr[0], sol->qr[1], sol->qr[2], sol->qr[3], sol->qr[4], sol->qr[5]);
            //printf("solution status: %u, valid sat: %d\n", sol->stat, sol->ns);
            // unsigned char buff[4096];
            // double rb[3];

            // outprcopts(buff, &prcopt);
            // printf("opt: %s\n", buff);

            // outsolheads(buff, &solopt);
            // printf("sol head: %s\n", buff);

            // outsols(buff, sol, rb, &solopt);
            // printf("sols: %s\n", buff);
            // printf("rb: %lf, %lf, %lf\n", rb[0], rb[1], rb[2]);
            // printf("msg: %s\n\n", rtk.errbuf);
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