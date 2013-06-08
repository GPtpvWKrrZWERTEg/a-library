#include <nata/nata_rcsid.h>
__rcsId("$Id: ltest.cpp 102 2013-05-30 15:20:16Z mzbjyza2mji4zgz $")

#include <nata/libnata.h>

#include <nata/Thread.h>
#include <nata/Process.h>
#include <nata/SignalThread.h>
#include <nata/Statistics.h>
#include <nata/CommandLineParameters.h>

#include <math.h>





class Worker: public Thread {
private:

    typedef void (*yieldProcT)(void);

    bool mUsePMtx;
    Mutex &mLock0;
    Mutex &mMtx;
    pthread_mutex_t &mPMtx;
    uint64_t mBegin;
    uint64_t mEnd;
    uint64_t mId;
    size_t mNTries;
    yieldProcT mYieldProc;

    pthread_mutex_t mDummyPMtx;
    Mutex mDummyMtx;

    static inline void
    sDummyYield(void) {
        return;
    }

    static inline void
    sYield(void) {
        yield();
    }

    inline int
    pmtxRun(void) {
        dbgMsg("Enter.\n");

        mLock0.lock();
        mLock0.unlock();

        size_t i;

        mBegin = NanoSecond::getCurrentTimeInNanos();
        for (i = 0; i < mNTries; i++) {
            (void)pthread_mutex_lock(&mPMtx);
            (void)pthread_mutex_unlock(&mPMtx);
            mYieldProc();
        }
        mEnd = NanoSecond::getCurrentTimeInNanos();

        dbgMsg("Leave.\n");

        return 0;
    }

    inline int
    mtxRun(void) {
        dbgMsg("Enter.\n");

        mLock0.lock();
        mLock0.unlock();

        size_t i;

        mBegin = NanoSecond::getCurrentTimeInNanos();
        for (i = 0; i < mNTries; i++) {
            mMtx.lock();
            mMtx.unlock();
            mYieldProc();
        }
        mEnd = NanoSecond::getCurrentTimeInNanos();

        dbgMsg("Leave.\n");

        return 0;
    }

protected:
    int
    run(void) {
        return (mUsePMtx == true) ? pmtxRun() : mtxRun();
    }

public:
    inline
    Worker(Mutex &l0, pthread_mutex_t &pmtx, uint64_t id, size_t nTries) :
        Thread(),
        mUsePMtx(true),
        mLock0(l0),
        mMtx(mDummyMtx),
        mPMtx(pmtx),
        mBegin(0),
        mEnd(0),
        mId(id),
        mNTries(nTries),
        mYieldProc(sDummyYield) {
    }

    inline
    Worker(Mutex &l0, Mutex &mtx, uint64_t id, size_t nTries) :
        Thread(),
        mUsePMtx(false),
        mLock0(l0),
        mMtx(mtx),
        mPMtx(mDummyPMtx),
        mBegin(0),
        mEnd(0),
        mId(id),
        mNTries(nTries),
        mYieldProc(sDummyYield) {
    }

    inline uint64_t
    getBeginTime(void) {
        return mBegin;
    }

    inline uint64_t
    getEndTime(void) {
        return mEnd;
    }

    inline void
    setCallYield(bool val) {
        mYieldProc = (val == true) ? sYield : sDummyYield;
    }

};





class Options: public CommandLineParameters {
private:
    size_t mNThds;
    size_t mNTries;
    size_t mNIters;
    bool mUsePMtx;
    bool mCallYield;
    bool mUsaged;

    static bool
    parseNumThreads(const char *arg, void *ctx) {
        bool ret = false;
        Options *o = (Options *)ctx;
        int64_t tmp;
        if (o->parseInt64(arg, tmp) == true) {
            if (tmp >= 1) {
                o->mNThds = tmp;
                ret = true;
            } else {
                fprintf(stderr, "%s: error: must be >= 1.\n",
                        o->commandName());
            }
        }
        return ret;
    }
    friend bool parseNumThreads(const char *arg, void *ctx);

    static bool
    parseNumIters(const char *arg, void *ctx) {
        bool ret = false;
        Options *o = (Options *)ctx;
        int64_t tmp;
        if (o->parseInt64(arg, tmp) == true) {
            if (tmp >= 1) {
                o->mNIters = tmp;
                ret = true;
            } else {
                fprintf(stderr, "%s: error: must be >= 1.\n",
                        o->commandName());
            }
        }
        return ret;
    }
    friend bool parseNumIters(const char *arg, void *ctx);

    static bool
    parseNumTries(const char *arg, void *ctx) {
        bool ret = false;
        Options *o = (Options *)ctx;
        int64_t tmp;
        if (o->parseInt64(arg, tmp) == true) {
            if (tmp >= 1) {
                o->mNTries = tmp;
                ret = true;
            } else {
                fprintf(stderr, "%s: error: must be >= 1.\n",
                        o->commandName());
            }
        }
        return ret;
    }
    friend bool parseNumTries(const char *arg, void *ctx);

    static bool
    parseUsePMtx(const char *arg, void *ctx) {
        (void)arg;
        Options *o = (Options *)ctx;
        o->mUsePMtx = true;
        return true;
    }
    friend bool parseUsePMtx(const char *arg, void *ctx);

    static bool
    parseCallYield(const char *arg, void *ctx) {
        (void)arg;
        Options *o = (Options *)ctx;
        o->mCallYield = true;
        return true;
    }
    friend bool parseCallYield(const char *arg, void *ctx);

    static bool
    doUsage(const char *arg, void *ctx) {
        (void)arg;
        Options *o = (Options *)ctx;
        o->usage();
        o->mUsaged = true;
        return true;
    }
    friend bool doUsage(const char *arg, void *ctx);

private:
    Options(const Options &obj);
    Options operator = (const Options &obj);


public:
    inline
    Options(const char *argv0) :
        CommandLineParameters(argv0),
        mNThds(1),
        mNTries(10000),
        mNIters(1000),
        mUsePMtx(false),
        mCallYield(false),
        mUsaged(false) {

        addParser("-n", parseNumThreads, (void *)this,
                  Parse_Next, Value_Integer, false, false,
                  "Specify # of threads (default: 1).");

        addParser("-t", parseNumTries, (void *)this,
                  Parse_Next, Value_Integer, false, false,
                  "Specify # of lock()/unlock() calls (default: 10000).");

        addParser("-i", parseNumIters, (void *)this,
                  Parse_Next, Value_Integer, false, false,
                  "Specify # of iterations (default: 1000).");

        addParser("-p", parseUsePMtx, (void *)this,
                  Parse_This, Value_Boolean, false, false,
                  "\tUse a raw pthread_mutex_t as a lock.");

        addParser("-y", parseCallYield, (void *)this,
                  Parse_This, Value_Boolean, false, false,
                  "\tYield after unlock() all the time.");

        addParser("-?", doUsage, (void *)this,
                  Parse_This, Value_Boolean, false, false,
                  "\tShows this.");
    }

    inline size_t
    getNumThreads(void) {
        return mNThds;
    }

    inline size_t
    getNumTries(void) {
        return mNTries;
    }

    inline size_t
    getNumIters(void) {
        return mNIters;
    }

    inline bool
    usePthreadMutex(void) {
        return mUsePMtx;
    }

    inline bool
    callYield(void) {
        return mCallYield;
    }

    inline bool
    isUsageCalled(void) {
        return mUsaged;
    }

};





volatile static bool doLoop = true;


static void
quitSignals(int sig) {

    (void)sig;

    dbgMsg("Enter.\n");

    doLoop = false;
}


static void
setupSignals(void) {
    SignalThread *st = new SignalThread();

    st->ignore(SIGPIPE);

    st->setHandler(SIGHUP, quitSignals);
    st->setHandler(SIGINT, quitSignals);
    st->setHandler(SIGQUIT, quitSignals);
    st->setHandler(SIGTERM, quitSignals);

    st->start(false, false);
}


static void
emitResult(Statistics<uint64_t> &s, size_t nThds) {
    uint64_t minT = s.min();
    uint64_t maxT = s.max();
    double avgT = s.average();
    double varT = s.variance();

    printf("\n\nOverall:\n");
    printf("min:\t" PF64s(10, u) " ns\n"
           "max:\t" PF64s(10, u) " ns\n"
           "avg:\t%10.4f ns\n"
           "sd:\t%10.4f ns\n", 
           minT, maxT, avgT, sqrt(varT));

    printf("\nPer thread:\n");
    printf("min:\t%10.4f ns\n"
           "max:\t%10.4f ns\n"
           "avg:\t%10.4f ns\n"
           "sd:\t%10.4f ns\n", 
           (double)minT / (double)nThds,
           (double)maxT / (double)nThds,
           avgT / nThds,
           sqrt(varT) / nThds);

    return;
}




    
int
main(int argc, const char * const argv[]) {
    bool ok = false;
    Mutex l0;
    Mutex mtx;
    pthread_mutex_t pmtx = PTHREAD_MUTEX_INITIALIZER;
    size_t i;
    size_t j;
    char nmBuf[32];
    uint64_t maxEnd;
    uint64_t minBegin;
    size_t nThds;
    size_t nTries;
    size_t nIters;
    bool usePMtx;
    bool callYield;
    Worker **w = NULL;
    Statistics<uint64_t> s;

    Options o(argv[0]);

    if (o.parse(argc - 1, (char * const *)argv + 1) != true) {
        goto done;
    } else if (o.isUsageCalled() == true) {
        ok = true;
        goto done;
    }

    nThds = o.getNumThreads();
    nTries = o.getNumTries();
    nIters = o.getNumIters();
    usePMtx = o.usePthreadMutex();
    callYield = o.callYield();

    w = (Worker **)malloc(sizeof(Worker *) * nThds);
    if (w == NULL) {
        goto done;
    }

    setupSignals();

    printf("Threads:\t" PF64(u) "\n"
           "Lock/Unlock:\t" PF64(u) " times/threads.\n"
           "Iterations:\t" PF64(u) " times.\n"
           "Mutex:\t\t%s\n"
           "Yield:\t\t%s\n\n",
           nThds, nTries, nIters,
           (usePMtx == true) ? "pthread_mutex_t" : "Mutex",
           (callYield == true) ? "Yes" : "No");

    for (i = 0; i < nThds; i++) {
        if (usePMtx == true) {
            w[i] = new Worker(l0, pmtx, i, nTries);
        } else {
            w[i] = new Worker(l0, mtx, i, nTries);
        }
        w[i]->setCallYield(callYield);
        snprintf(nmBuf, sizeof(nmBuf), "Worker " PFSzs(4, u), i + 1);
        w[i]->setName((const char *)nmBuf);
    }

    for (j = 0; j < nIters && doLoop == true; j++) {
        fprintf(stderr, "Iter: " PF64s(10, u) "\r", j);

        l0.lock();

        for (i = 0; i < nThds; i++) {
            w[i]->start();
        }

        l0.unlock();

        maxEnd = 0;
        minBegin = ULLONG_MAX;

        for (i = 0; i < nThds && doLoop == true; i++) {
            w[i]->wait();
            uint64_t tmp = w[i]->getBeginTime();
            if (minBegin > tmp) {
                minBegin = tmp;
            }
            tmp = w[i]->getEndTime();
            if (maxEnd < tmp) {
                maxEnd = tmp;
            }
        }
        
        s.record((maxEnd - minBegin) / nTries);
    }

    for (i = 0; i < nThds; i++) {
        delete w[i];
    }
    free((void *)w);

    emitResult(s, nThds);

    if (doLoop == true) {
        ok = true;
    }

    done:
    return (ok == true) ? 0 : 1;
}
