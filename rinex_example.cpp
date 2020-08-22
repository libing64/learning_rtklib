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

int main(int argc, char **argv)
{
    gtime_t t0 = {0}, ts = {0}, te = {0};
    char file1[] = "../data/07590920.05o";
    char file2[] = "../data/07590920.05n";

    obs_t obs = {0};
    nav_t nav = {0};
    sta_t sta = {""};

    cout << "readin rinex file: " << file2 << endl;
    cout << "================================" << endl;
    readrnxt(file2, 1, ts, te, 0.0, "", &obs, &nav, &sta);

    dumpnav(&nav);

    readrnxt(file1, 1, t0, t0, 0.0, "", &obs, &nav, &sta);
    dumpobs(&obs);
    dumpsta(&sta);

    free(obs.data);
    return 0;
}
