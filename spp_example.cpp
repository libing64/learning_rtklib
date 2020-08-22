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

        assert(1 <= obs->data[i].sat && obs->data[i].sat <= 32);
        assert(timediff(obs->data[i].time, time) >= -DTTOL);

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

static void dumpopt(const prcopt_t *opt)
{
    printf("mode: %d\n", opt->mode);
    printf("soltype: %d\n", opt->soltype);
    printf("nf: %d\n", opt->nf);
    printf("navsys: %d\n", opt->navsys);
    printf("elmin: %lf\n", opt->elmin);
    printf("sateph: %d\n", opt->sateph);
    printf("modear: %d\n", opt->modear);
    printf("glomodear: %d\n", opt->glomodear);
    printf("bdsmodear: %d\n", opt->bdsmodear);
    printf("maxout: %d\n", opt->maxout);
    printf("minlock: %d\n", opt->minlock);
    printf("minfix: %d\n", opt->minfix);
    printf("ionoopt: %d\n", opt->ionoopt);
    printf("tropopt: %d\n", opt->tropopt);
    printf("dynamics: %d\n", opt->dynamics);
    printf("tidecorr: %d\n", opt->tidecorr);
    printf("niter: %d\n", opt->niter);
    printf("codesmooth: %d\n", opt->codesmooth);

    printf("std: %lf,%lf,%lf\n", opt->std[0], opt->std[1], opt->std[2]);
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
void pntpos_process(obs_t *obs, nav_t *nav, prcopt_t *opt)
{
    gtime_t time = {0};
    sol_t sol;
    char msg[128];

    dumpopt(opt);
    int i = 0, j = 0;
    double ra[6];
    int rcv = 1;
    int n = 0, m = 0;
    for (i = 0; i < 3; i++)
        ra[i] = 0.0;

    for (int i = 0; i < NSATGPS; i++)
    {
        nav->lam[i][0] = CLIGHT / FREQ1;
        nav->lam[i][1] = CLIGHT / FREQ2;
        nav->lam[i][2] = CLIGHT / FREQ5;
    }

    for (int i = 0; (m = nextobsf(obs, &i, rcv)) > 0; i += m)
    {
        int ret = pntpos(&obs->data[i], m, nav, opt, &sol, NULL, NULL, msg);
        if (ret == 1)//1：OK, 0: error
        {
            double ep[6] = {0};
            time2epoch(sol.time, ep);
            printf("%.0lf,%.0lf,%.0lf,%.0lf,%.0lf,%.0lf,%lf,%lf,%lf,%lf,%lf,%lf,\n", ep[0], ep[1], ep[2], ep[3], ep[4], ep[5],
                   sol.rr[0], sol.rr[1], sol.rr[2], sol.rr[3], sol.rr[4], sol.rr[5]);
        }
        else
        {
            printf("ret: %d, msg:%s\n", ret, msg);
        }
    }
}

int main(int argc, char **argv)
{
    gtime_t t0 = {0}, ts = {0}, te = {0};
    char file1[1024] = "../data/rinex/daej229a00.20n";
    char file2[1024] = "../data/rinex/daej229a00.20o";

    if (argc == 3)
    {
        strcpy(file1, argv[1]);
        strcpy(file2, argv[2]);
    }

    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {""};

    readrnxt(file1, 1, ts, te, 0.0, "", &obs, &nav, &sta);
    readrnxt(file2, 1, t0, t0, 0.0, "", &obs, &nav, &sta);

    traceopen("spp.trace");
    tracelevel(4);
    tracenav(2, &nav);

    prcopt_t opt = prcopt_default;
    pntpos_process(&obs, &nav, &opt);

    traceclose();
    free(obs.data);
    return 0;
}