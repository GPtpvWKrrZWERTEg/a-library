/*
 * $Id: NTimer.h 60 2011-12-14 21:28:38Z mzbjyza2mji4zgz $
 */
#ifndef __NTIMER_H__
#define __NTIMER_H__


#include <nata/NanoSecond.h>



class NTimer {


private:
    __rcsId("$Id: NTimer.h 60 2011-12-14 21:28:38Z mzbjyza2mji4zgz $");


    NanoSecond mStart;
    NanoSecond mStop;

    

public:


    inline
    NTimer(void) {
        mStart = CURRENT_TIME_IN_NANOS;
        mStop = mStart;
    }


    inline
    NTimer(const NTimer &obj) {
        mStart = obj.mStart;
        mStop = obj.mStop;
    }


    inline NTimer &
    operator = (const NTimer &obj) {
        mStart = obj.mStart;
        mStop = obj.mStop;
        return *this;
    }


    inline void
    start(void) {
        mStart = CURRENT_TIME_IN_NANOS;
        mStop = mStart;
    }


    inline void
    restart(void) {
        start();
    }


    inline void
    stop(void) {
        mStop = CURRENT_TIME_IN_NANOS;
    }


    inline uint64_t
    started(void) {
        return (uint64_t)mStart;
    }


    inline uint64_t
    elapsed(void) {
        return mStop - mStart;        
    }


    inline uint64_t
    measure(void) {
        stop();
        return elapsed();
    }





    static inline uint64_t
    now(void) {
        return NanoSecond::getCurrentTimeInNanos();
    }


    static inline void
    sleep(uint64_t nsec) {
        NanoSecond::nanoSleep(nsec);
    }
};


#endif // ! __NTIMER_H__
