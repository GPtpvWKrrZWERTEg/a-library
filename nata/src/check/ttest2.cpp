#include <nata/nata_rcsid.h>


__rcsId("$Id: ttest2.cpp 83 2012-04-04 22:13:08Z mzbjyza2mji4zgz $")


#include <nata/Statistics.h>
#include <nata/Thread.h>
#include <nata/NanoTimer.h>
#include <nata/Statistics.h>
#include <nata/PeriodicAlarm.h>

#include <math.h>

static PeriodicAlarm pa;


class testThread: public Thread {
private:
    
    PeriodicAlarmHandle *mHndl;

    inline int
    run(void) {
        uint64_t val;
        uint64_t diff, intv;
        uint64_t oTime = NanoSecond::getCurrentTimeInNanos();
        uint64_t curTime = oTime;
        bool isFirst = true;
        Statistics<uint64_t> diffS, intvS;

        while (mHndl->wait(val) != false && 
               val != 0LL) {
            curTime = NanoSecond::getCurrentTimeInNanos();
            if (isFirst == false) {
                diff = curTime - val;
                intv = curTime - oTime;
                diffS.record(diff);
                intvS.record(intv);
                dbgMsg("interval " PF64(u) " diff " PF64(u) "\n",
                       intv, diff);
            } else {
                isFirst = false;
            }
            oTime = curTime;
        }

        dbgMsg("INTV: " PF64(u) ": %f/%f/%f/%f\n",
               intvS.recorded(),
               (double)intvS.min() / 1000000.0,
               intvS.average() / 1000000.0,
               (double)intvS.max() / 1000000.0,
               sqrt(intvS.variance()) / 1000000.0);

        dbgMsg("DIFF: " PF64(u) ": %f/%f/%f/%f\n",
               diffS.recorded(),
               (double)diffS.min() / 1000000.0,
               diffS.average() / 1000000.0,
               (double)diffS.max() / 1000000.0,
               sqrt(diffS.variance()) / 1000000.0);

        pa.releaseAlarm(mHndl);
        mHndl = NULL;

        return 0;
    }

public:
    inline
    testThread(void) :
        Thread(),
        mHndl(NULL) {
        mHndl = pa.acquireAlarm();
        if (mHndl == NULL) {
            fatal("Can't acquire an alarm.\n");
        }
    }

    inline
    ~testThread(void) {
        if (mHndl != NULL) {
            pa.releaseAlarm(mHndl);
            mHndl = NULL;
        }
    }
};


int
main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    nata_InitializeLogger(emit_Syslog, "hoehoe", false, false, 1);

    NanoSecond::initialize(100000);

#if 1
    uint64_t now1, now0;
    for (int i = 0; i < 10; i++) {
        now0 = NanoSecond::getCurrentTimeInNanos();
#if defined(NATA_API_WIN32API)
        Sleep(1000);
#elif defined(NATA_API_POSIX)
        NanoSecond::nanoSleep(1000000000LL);
#endif // NATA_API_WIN32API || NATA_API_POSIX
        now1 = NanoSecond::getCurrentTimeInNanos();
        dbgMsg("diff: " PF64(u) "\n", now1 - now0);
    }

    Statistics<uint64_t> wow;
    uint64_t a0, a1, ad, b0, b1;

    dbgMsg("start.\n");

    b0 = NanoSecond::getCurrentTimeInNanos();    
    for (int i = 0; i < 600; i++) {
        a0 = NanoSecond::getCurrentTimeInNanos();
        NanoSecond::nanoSleep(16666666LL);
        //NanoSecond::nanoSleep(33333333LL);
        a1 = NanoSecond::getCurrentTimeInNanos();
        ad = a1 - a0;
        dbgMsg(PF64(u) " ns\n", ad);
        wow.record(ad);
    }
    b1 = NanoSecond::getCurrentTimeInNanos();

    dbgMsg("ittr: " PF64(u) " \n", wow.recorded());
    dbgMsg("%f / %f / %f / %f\n",
           (double)wow.min() / 1000000.0,
           wow.average() / 1000000.0,
           (double)wow.max() / 1000000.0,
           sqrt(wow.variance()) / 1000000.0);

    dbgMsg("total: " PF64(u) " ns\n", b1 - b0);

    return 0;
#endif

    testThread tt0, tt1;
    tt0.start();
    tt1.start();

    pa.start();

    NanoSecond::nanoSleep(5LL * 1000LL * 1000LL * 1000LL);

    pa.stop();
    pa.wait();

    NanoSecond t0 = 0LL;
    NanoSecond tNow = CURRENT_TIME_IN_NANOS;

    NanoSecond t1 = tNow - t0;
    NanoSecond t2;

    fprintf(stderr, PF64(u) ", " PF64(u) ", " PF64(u) "\n",
            (uint64_t)t0, (uint64_t)tNow, (uint64_t)t1);

    t2 = t0;

    t1 = t1 + 1000LL;
    t1 = 1000LL + t1;
    t1 = t1 - 1LL;

    t1 = t1 / 1LL;
    t1 = t1 * 1LL;

    fprintf(stderr, PF64(u) ", " PF64(u) ", " PF64(u) "\n",
            (uint64_t)t0, (uint64_t)tNow, (uint64_t)t1);


    if (t0 == t2) {
        fprintf(stderr, "nownow\n");
    }
    if (t0 == 0LL) {
        fprintf(stderr, "0LL\n");
    }

    struct timeval tv;

    tv = t1.toTimeval(tv);

    fprintf(stderr, "%ld, %ld\n", tv.tv_sec, tv.tv_usec);

    tv = (struct timeval)t1;

    fprintf(stderr, "%lu, %lu\n", tv.tv_sec, tv.tv_usec);


    NanoTimer nt0;

    nt0.start();

    NanoSecond::nanoSleep(2 * 1000 * 1000);

    nt0.stop();

    fprintf(stderr, PF64(u) ", " PF64(u) "\n", nt0.started(), nt0.elapsed());

    return 0;
}
