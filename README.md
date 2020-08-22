# RTKLIB学习
* rinex文件解析
* 根据广播星历估计卫星位置
* 单点定位
* 差分GPS定位
* rtk定位
* PPP定位


# 2. rinex文件解析

## 2.1 Rinex 格式
Rinex文件一ASCII码的方式存储, 主要分为4类，
* Observation Data File 观测数据文件
* Navigation Data File 导航数据文件(用于星历计算)
* Meteorological Data File 气象数据文件(用于时间补偿)

## 2.2 文件命名格式

文件命名格式 'ssssdddf.yyt'

例如 onsa2240.08o

* ssss 4字符基站名
* ddd  一年的第ddd天
* f    一天内的第f个文件
* yy   年份
* t    文件类型
  * o 观测文件
  * n 导航文件
  * m 气象文件
  * g 格罗纳斯导航文件

## 2.3 文件头和数据
Rinex文件分为文件头和数据两部分，文件头指明了基本信息，数据段是一条条的观测记录.
observation type 观测类型
  * C : pseudorange  伪距
  * L : carrier phase 载波相位
  * D : doppler 多普勒
  * S : signal strengh 信号强度


每条观测数据有指明观测类型和频段，主要以下几种
* C1    : C/A码在L1频段的伪距测量
* L1 L2 : 在L1/L2频段的相位测量
* P1 P2 : P码在L1/L2频段的伪距测量
* D1 D2 : 在L1/L2频段的多普勒频率
* T1 T2 : 在150MHz(T1)和400MHz(T2)的传输集成多普勒

数据段的每天观测有记录历元时刻，历元标志，卫星ID等
```
 08  5 27  0  0  0.0000000  0  8G 6G21G29G30G31G 3G24G16
 114882249.33248  89518626.24848  21861373.4054   21861376.2644   21861373.5894
       238.000         204.000
 110254184.33348  85912345.81348  20980679.0014   20980680.5424   20980679.8634
       249.000         223.000
 120713732.89848  94062638.76348  22971066.7834   22971069.3844   22971066.3794
       231.000         194.000
 127596858.63848  99426108.12048  24280886.5204   24280890.7184   24280887.1834
       218.000         159.000
 113742340.75548  88630389.25948  21644452.3124   21644453.7974   21644451.9784
       245.000         216.000
 122838795.24448  95718531.73348  23375451.5884   23375454.5834   23375451.0024
       216.000         162.000
 117616989.64848  91649585.97448  22381782.3054   22381787.0184   22381782.7914
       238.000         201.000
 111208337.76148  86655834.69148  21162252.8314   21162255.1284   21162253.5234
       248.000         220.000
```

* 08  5 27  0  0  0.0000000 : 历元时刻
* 0                         : 历元标志， 0:正常，1：异常
* 8G 6G21G29G30G31G 3G24G16 : 8代表有8个观测，后面是每个卫星的ID，前缀G代表GPS(G 3,代表G03？ 不带补0的？)
下面每行就是每个卫星的观测数据

## 2.4 rinex文件下载
下载地址 
* ftp://nfs.kasi.re.kr/gps/data/daily
* ftp://cddis.gsfc.nasa.gov/highrate/2020/188
* ftp://www.igs.org/pub/

访问FTP数据需要通过ftp客户端, 比如nautilus或者FileZilla

ftp客户端中输入ftp地址 例如 ftp://nfs.kasi.re.kr/gps/data/daily/2020/001
可以直接从中获取rinex文件

其它FPT客户端也都可以
https://linuxconfig.org/how-to-install-ftp-client-for-ubuntu-18-04-bionic-beaver-linux

## 2.5 rinex文件解析
代码参考 rinex_example.cpp

```
./rinex_example

...
nav : n=162
2005/04/02 02:00:00.000 :  1    2005/04/02 02:00:00 2005/04/02 00:19:36 140 396  0
2005/04/02 00:00:00.000 :  3    2005/04/02 00:00:00 2005/04/01 22:00:18  83 595  0
2005/04/02 02:00:00.000 :  3    2005/04/02 02:00:00 2005/04/02 00:00:18  84 340  0
2005/04/02 02:00:00.000 :  4    2005/04/02 02:00:00 2005/04/02 00:41:18 149 149  0
2005/04/02 00:00:00.000 :  7    2005/04/02 00:00:00 2005/04/01 23:22:42  73  73  0
2005/04/02 02:00:00.000 :  7    2005/04/02 02:00:00 2005/04/02 00:00:18  74  74  0
2005/04/02 00:00:00.000 :  8    2005/04/02 00:00:00 2005/04/01 22:00:18 176 688  0

...
obs : n=948
2005/04/02 00:00:00.000 :  3  1  55923622.160  43647388.242  24767686.375  24767684.822  0 0
2005/04/02 00:00:00.000 :  7  1   -691177.898   -537007.140  24361933.475  24361930.599  0 0
2005/04/02 00:00:00.000 :  8  1  17984490.035  14018464.809  23407378.219  23407374.320  0 0
2005/04/02 00:00:00.000 : 11  1   7712103.227   6019854.642  20311445.258  20311439.442  0 0
2005/04/02 00:00:00.000 : 19  1  36724126.590  28621450.827  22613015.950  22613010.110  0 0
2005/04/02 00:00:00.000 : 20  1  -5764048.758  -4479034.461  21565852.190  21565847.229  0 0
2005/04/02 00:00:00.000 : 24  1  -2292750.457  -1749426.201  22276378.821  22276375.748  0 0
2005/04/02 00:00:00.000 : 28  1  -5448227.324  -4238014.209  21543408.487  21543403.046  0 0
```

# 3. 根据广播星历计算卫星位置
代码参考 satpos_example.cpp


```
./satpos_example
...

2020.000000,8.000000,16.000000,0.000000,1.000000,0.000000
01     60  -13579942.467    9771755.078   20305999.967      33959.061
03     60  -23371334.105   12368554.699    1740200.870    -271150.525
07     60   -6116449.145   24004937.051   -8551534.906    -351050.818
08     60  -23864836.161   -4263902.654   11160344.637     -44598.650
11     60  -17062646.729     372106.871   19799556.205    -191622.733
17     60    9683226.017   19060992.875   16213366.523     311619.017
19     60   16019360.466   18952228.716    9250491.532    -117914.607
22     60  -23419416.229    7832245.251    9782836.952    -763790.272
28     60   -1288194.531   16544638.575   21353310.065     689127.930
30     60     477556.699   26397274.483    1469893.733    -283002.660
```
第一行是时间, 2020年8月16日，0时1分0秒
后面几行是卫星sn, 时间，位置三维位置，卫星钟差(纳秒)

# 4. 单点定位
已知N个卫星的位置和卫星到接收机的伪距，求解接收机的位置和钟差，求解方法比较简单，标准的最小二乘或者优化方法都可以解，公式推导参考Basics of the GPS Technique, 不过最麻烦的是各种误差补偿，比如电离层误差、对流层误差等.

代码参考spp_example.cpp
注意调用之前要给lambda赋值(波长)，这个问题当时我查了一天.
```
for (int i = 0; i < NSATGPS; i++)
{
    nav->lam[i][0] = CLIGHT / FREQ1;
    nav->lam[i][1] = CLIGHT / FREQ2;
    nav->lam[i][2] = CLIGHT / FREQ5;
}
```

```
./spp_example
...

2020,8,15,23,59,60,-3120046.686870,4084621.475955,3764034.709503,0.000000,0.000000,0.000000,
2020,8,16,0,0,1,-3120047.650995,4084622.776844,3764034.879003,0.000000,0.000000,0.000000,
2020,8,16,0,0,2,-3120048.149014,4084623.768534,3764035.318619,0.000000,0.000000,0.000000,
2020,8,16,0,0,3,-3120047.890838,4084622.629507,3764034.940171,0.000000,0.000000,0.000000,
2020,8,16,0,0,4,-3120046.991411,4084621.607839,3764033.861034,0.000000,0.000000,0.000000,
2020,8,16,0,0,5,-3120047.497998,4084622.579997,3764034.913619,0.000000,0.000000,0.000000,
2020,8,16,0,0,6,-3120046.906680,4084622.429260,3764034.066605,0.000000,0.000000,0.000000,
2020,8,16,0,0,7,-3120048.192236,4084623.991768,3764035.448642,0.000000,0.000000,0.000000,
```
打印的是时间、位置、速度，由于obs文件中没有包含doppler观测，所以这里没有解算速度. 


# 5. 差分定位
TODO
代码参考 dpp_example.cpp

# 6. rtk定位
定位精度可以达到mm, 不过只能计算相对位置, 需要额外的基站(base station)，而且基站和接收机(rover station)之间需要通信链路.
rtk定位的重点在于两次差分, 第一次差分是站间差分(base station 和 rover station), 能够消除卫星钟差, 电离层误差、对流层误差等，第二次差分是星间差分, 能够消除接收机钟差. 公式推导参考 Basics of the GPS Technique.

代码参考rtk_example.cpp

```
./rtk_example
...

station pos: -3978242.434800,3382841.171500,3649902.766700
(0,17)/1987, type: 0, pos: 2022.774943,-468.630735,2610.284897
(17,17)/1987, type: 0, pos: 2022.777023,-468.636038,2610.280449
(34,17)/1987, type: 0, pos: 2022.775807,-468.634462,2610.280089
(51,17)/1987, type: 0, pos: 2022.769654,-468.626963,2610.285410
(68,17)/1987, type: 0, pos: 2022.770668,-468.633941,2610.287829
(85,17)/1987, type: 0, pos: 2022.772624,-468.633807,2610.285683
(102,17)/1987, type: 0, pos: 2022.764647,-468.624600,2610.291085
(119,17)/1987, type: 0, pos: 2022.762379,-468.620144,2610.292037
(136,17)/1987, type: 0, pos: 2022.765727,-468.621999,2610.294079
(153,17)/1987, type: 0, pos: 2022.771617,-468.636478,2610.285908
(170,17)/1987, type: 0, pos: 2022.772370,-468.631508,2610.292070
```

# 7. PPP精密单点定位
精密星历文件下载 ftp://cddis.gsfc.nasa.gov/pub/gps/products/2119
* igr 快速星历(IGS rapid ephemeris)
* igu 超快速星历(IGS ultra rapid ephemeris)
* igf 事后精密星历(IGS final precise ephemeris)
其中IGF轨道精度最高，优于3cm
IGR轨道精度次之, 优于4cm
IGU实测部分精度优于5cm, 预报部分精度较低，约为10cm


## 7.1 根据精密轨道参数估计卫星位置
代码参考ppp_satpos_example.cpp

```
./ppp_satpos_example

2020.000000,8.000000,16.000000,0.000000,0.000000,0.000000
01      0  -13573837.896    9929603.849   20232286.958      33959.136  -13573838.559    9929604.454   20232287.850      33954.038
03      0  -23374402.912   12389243.851    1549017.642    -271149.817  -23374403.920   12389244.701    1549017.042    -271154.978
07      0   -6096857.839   24073097.783   -8375705.752    -351050.043   -6096858.614   24073097.660   -8375705.373    -351058.406
08      0  -23796056.904   -4212673.114   11324081.130     -44598.670  -23796058.543   -4212674.400   11324080.895     -44607.385
11      0  -16997160.108     517415.986   19853350.492    -191623.045  -16997160.908     517415.933   19853351.799    -191625.199
17      0    9786437.220   19118562.789   16079790.071     311618.481    9786436.435   19118562.839   16079789.874     311611.203
19      0   16084929.559   18984451.661    9074928.465    -117914.856   16084929.215   18984451.792    9074928.575    -117921.838
22      0  -23466875.757    7891092.688    9616095.842    -763790.469  -23466876.286    7891092.910    9616094.929    -763797.522
28      0   -1147008.702   16482062.577   21410400.275     689127.804   -1147009.813   16482063.461   21410401.055     689126.488
```
第一行打印的是时间，后面分别是广播星历计算的卫星位置和钟差(3-6),精密星历计算的卫星位置和钟差(7-10)


## 7.2 PPP精密定位
代码参考 ppp_example.cpp

```
./ppp_example
...

8396/8486, pos: -3120047.547767,4084621.477668,3764033.498128
8406/8486, pos: -3120047.547820,4084621.478091,3764033.498517
8416/8486, pos: -3120047.547898,4084621.478066,3764033.498457
8426/8486, pos: -3120047.547721,4084621.478756,3764033.498257
8436/8486, pos: -3120047.547051,4084621.478064,3764033.497623
8446/8486, pos: -3120047.547242,4084621.477836,3764033.497747
8456/8486, pos: -3120047.547520,4084621.477508,3764033.497979
8466/8486, pos: -3120047.546602,4084621.476270,3764033.497378
8476/8486, pos: -3120047.546458,4084621.475280,3764033.497483
```
ppp收敛速度非常慢, 看最后几次的计算结果，精度还是很高的

# 8. 编译&运行
## 8.1 rtklib 编译&安装
下载rtklib源码, 编译成静态库并安装
```
git clone https://github.com/tomojitakasu/RTKLIB.git
cd RTKLIB/src
``` 

在RTKLIB/src目录下添加CMakeLists.txt文件

```
cmake_minimum_required(VERSION 3.1)
project(rtklib)

add_definitions(-DTRACE)#enable trace in rtklib

set(RTKLIB_SRC convkml.c convrnx.c datum.c download.c ephemeris.c geoid.c ionex.c lambda.c options.c pntpos.c postpos.c ppp_ar.c ppp.c preceph.c qzslex.c rcvraw.c rinex.c rtcm2.c rtcm3.c rtcm3e.c rtcm.c rtkcmn.c rtkpos.c rtksvr.c sbas.c solution.c stream.c streamsvr.c tle.c)
add_library(rtklib STATIC ${RTKLIB_SRC})

install(TARGETS rtklib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES rtklib.h DESTINATION include)
```

```
mkdir build
cd build
cmake ..
make
sudo make install
```
此时rtklib已经作为静态库被安装在系统目录下， 基于rtklib二次开发也不用带源码了，直接引用静态库即可.
注意-DTRACE的做用是打开trace开关, 遇到问题调试起来会快不少. 
添加如下几行代码， 就可以使能trace功能
```
traceopen("rtklib.trace");
tracelevel(5);
//////////////////////////////
////////code//////////////
//////////////////////////////
traceclose();
```
你的目录下会多一个rtklib.trace的文件, 记录了代码执行的关键路径.


## 8.2 示例代码 编译&运行
```
cd rtklib_example
mkdir build
cd build
cmake ..
make
./rinex_example
./satpos_example
./spp_example
./rtk_example
./ppp_satpos_example
./ppp_example
```

# 9 rtklib 常用函数

## 9.1 整周模糊度计算
```
/* integer ambiguity resolution ----------------------------------------------*/
int lambda(int n, int m, const double *a, const double *Q, double *F, double *s);
```                    

## 9.2 单点定位
```
/* standard positioning ------------------------------------------------------*/
int pntpos(const obsd_t *obs, int n, const nav_t *nav,
                    const prcopt_t *opt, sol_t *sol, double *azel,
                    ssat_t *ssat, char *msg);
```

## 9.3 rtk定位
```
/* precise positioning -------------------------------------------------------*/
void rtkinit(rtk_t *rtk, const prcopt_t *opt);
void rtkfree(rtk_t *rtk);
int rtkpos(rtk_t *rtk, const obsd_t *obs, int nobs, const nav_t *nav);
int rtkopenstat(const char *file, int level);
void rtkclosestat(void);
```

## 9.4 PPP精度定位
```
/* precise point positioning -------------------------------------------------*/
void pppos(rtk_t *rtk, const obsd_t *obs, int n, const nav_t *nav);
int pppamb(rtk_t *rtk, const obsd_t *obs, int n, const nav_t *nav,
                    const double *azel);
int pppnx(const prcopt_t *opt);
void pppoutsolstat(rtk_t *rtk, int level, FILE *fp);
void windupcorr(gtime_t time, const double *rs, const double *rr,
                        double *phw);
```
## 9.5 数据后处理
```
/* post-processing positioning -----------------------------------------------*/
int postpos(gtime_t ts, gtime_t te, double ti, double tu,
                    const prcopt_t *popt, const solopt_t *sopt,
                    const filopt_t *fopt, char **infile, int n, char *outfile,
                    const char *rov, const char *base);
```

# 10 rtklib 宏定义

## solution status
```
#define SOLQ_NONE 0   /* solution status: no solution */
#define SOLQ_FIX 1    /* solution status: fix */
#define SOLQ_FLOAT 2  /* solution status: float */
#define SOLQ_SBAS 3   /* solution status: SBAS */
#define SOLQ_DGPS 4   /* solution status: DGPS/DGNSS */
#define SOLQ_SINGLE 5 /* solution status: single */
#define SOLQ_PPP 6    /* solution status: PPP */
#define SOLQ_DR 7     /* solution status: dead reconing */
#define MAXSOLQ 7     /* max number of solution status */
```

## solution type
```
#define SOLF_LLH 0  /* solution format: lat/lon/height */
#define SOLF_XYZ 1  /* solution format: x/y/z-ecef */
#define SOLF_ENU 2  /* solution format: e/n/u-baseline */
#define SOLF_NMEA 3 /* solution format: NMEA-183 */
#define SOLF_GSIF 4 /* solution format: GSI-F1/2/3 */
```


## positioning mode
```
#define PMODE_SINGLE 0     /* positioning mode: single */
#define PMODE_DGPS 1       /* positioning mode: DGPS/DGNSS */
#define PMODE_KINEMA 2     /* positioning mode: kinematic */
#define PMODE_STATIC 3     /* positioning mode: static */
#define PMODE_MOVEB 4      /* positioning mode: moving-base */
#define PMODE_FIXED 5      /* positioning mode: fixed */
#define PMODE_PPP_KINEMA 6 /* positioning mode: PPP-kinemaric */
#define PMODE_PPP_STATIC 7 /* positioning mode: PPP-static */
#define PMODE_PPP_FIXED 8  /* positioning mode: PPP-fixed */
```
PMODE_KINEMA 和 PMODE_FIXED怎么理解？


## time system
```
#define TIMES_GPST 0 /* time system: gps time */
#define TIMES_UTC 1  /* time system: utc */
#define TIMES_JST 2  /* time system: jst */
```
## ionosphere option 电离层改正参数
```
#define IONOOPT_OFF 0  /* ionosphere option: correction off */
#define IONOOPT_BRDC 1 /* ionosphere option: broadcast model */
#define IONOOPT_SBAS 2 /* ionosphere option: SBAS model */
#define IONOOPT_IFLC 3 /* ionosphere option: L1/L2 or L1/L5 iono-free LC */
#define IONOOPT_EST 4  /* ionosphere option: estimation */
#define IONOOPT_TEC 5  /* ionosphere option: IONEX TEC model */
#define IONOOPT_QZS 6  /* ionosphere option: QZSS broadcast model */
#define IONOOPT_LEX 7  /* ionosphere option: QZSS LEX ionospehre */
#define IONOOPT_STEC 8 /* ionosphere option: SLANT TEC model */
```


## troposphere option: 对流层改正参数
#define TROPOPT_OFF 0  /* troposphere option: correction off */
#define TROPOPT_SAAS 1 /* troposphere option: Saastamoinen model */
#define TROPOPT_SBAS 2 /* troposphere option: SBAS model */
#define TROPOPT_EST 3  /* troposphere option: ZTD estimation */
#define TROPOPT_ESTG 4 /* troposphere option: ZTD+grad estimation */
#define TROPOPT_COR 5  /* troposphere option: ZTD correction */
#define TROPOPT_CORG 6 /* troposphere option: ZTD+grad correction */

## ephemeris option 星历选项
```
#define EPHOPT_BRDC 0   /* ephemeris option: broadcast ephemeris */
#define EPHOPT_PREC 1   /* ephemeris option: precise ephemeris */
#define EPHOPT_SBAS 2   /* ephemeris option: broadcast + SBAS */
#define EPHOPT_SSRAPC 3 /* ephemeris option: broadcast + SSR_APC */
#define EPHOPT_SSRCOM 4 /* ephemeris option: broadcast + SSR_COM */
#define EPHOPT_LEX 5    /* ephemeris option: QZSS LEX ephemeris */
```

## AR mode
```
#define ARMODE_OFF 0       /* AR mode: off */
#define ARMODE_CONT 1      /* AR mode: continuous */
#define ARMODE_INST 2      /* AR mode: instantaneous */
#define ARMODE_FIXHOLD 3   /* AR mode: fix and hold */
#define ARMODE_PPPAR 4     /* AR mode: PPP-AR */
#define ARMODE_PPPAR_ILS 5 /* AR mode: PPP-AR ILS */
#define ARMODE_WLNL 6      /* AR mode: wide lane/narrow lane */
#define ARMODE_TCAR 7      /* AR mode: triple carrier ar */
```

## SBAS option:
```
#define SBSOPT_LCORR 1 /* SBAS option: long term correction */
#define SBSOPT_FCORR 2 /* SBAS option: fast correction */
#define SBSOPT_ICORR 4 /* SBAS option: ionosphere correction */
#define SBSOPT_RANGE 8 /* SBAS option: ranging */
```

## stream type:
```
#define STR_NONE 0     /* stream type: none */
#define STR_SERIAL 1   /* stream type: serial */
#define STR_FILE 2     /* stream type: file */
#define STR_TCPSVR 3   /* stream type: TCP server */
#define STR_TCPCLI 4   /* stream type: TCP client */
#define STR_UDP 5      /* stream type: UDP stream */
#define STR_NTRIPSVR 6 /* stream type: NTRIP server */
#define STR_NTRIPCLI 7 /* stream type: NTRIP client */
#define STR_FTP 8      /* stream type: ftp */
#define STR_HTTP 9     /* stream type: http */
```


## LLI

```
#define LLI_SLIP 0x01   /* LLI: cycle-slip */
#define LLI_HALFC 0x02  /* LLI: half-cycle not resovled */
#define LLI_BOCTRK 0x04 /* LLI: boc tracking of mboc signal */
#define LLI_HALFA 0x40  /* LLI: half-cycle added */
#define LLI_HALFS 0x80  /* LLI: half-cycle subtracted */
```