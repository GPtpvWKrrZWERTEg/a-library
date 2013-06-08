//#define DO_COMPLETION

#include <nata/nata_rcsid.h>
__rcsId("$Id: ttest.cpp 99 2013-01-25 17:09:27Z mzbjyza2mji4zgz $")

#include <nata/libnata.h>

#include <nata/Process.h>
#include <nata/ProcessJanitor.h>

#include <nata/Thread.h>
#include <nata/SignalThread.h>
#include <nata/SynchronizedMap.h>

#ifdef DO_COMPLETION
#include <nata/Completion.h>
#endif // DO_COMPLETION

#include <nata/IOHandle.h>

//#include <string>

#include <nata/nata_perror.h>

#include <nata/CircularBuffer.h>

#include <nata/CommandLineParameters.h>

class testElement {
private:
    uint8_t *mBuf;
    size_t mBufLen;


public:
    testElement(size_t len) :
        mBuf(NULL),
        mBufLen(len) {
        mBuf = (uint8_t *)malloc(len);
        if (mBuf == NULL) {
            fatal("Can't allocate a buffer.\n");
        }
    }

    ~testElement(void) {
        if (mBuf != NULL) {
            (void)free((void *)mBuf);
        }
    }
};


class testBuffer: public CircularBuffer<testElement *> {
private:
    static bool
    allocationHook(testElement **ptr, void *arg) {
        (void)arg;
        *ptr = new testElement(32);
        return (*ptr != NULL) ? true : false;
    }

    static void
    destructionHook(testElement *ptr, void *arg) {
        (void)arg;
        if (ptr != NULL) {
            delete ptr;
        }
    }

public:
    testBuffer(size_t len) :
        CircularBuffer<testElement *>(len) {
        setAllocationHook(allocationHook, NULL);
        setDestructionHook(destructionHook, NULL);
    }
};


class MyProcess: public Process {
protected:
    int
    runChild() {
        char buf[1024];

        fprintf(stdout, "child stdout: I'm child, %d.\n", fileno(stdout));
        fprintf(stderr, "child stderr: I'm child, %d.\n", fileno(stderr));
        fflush(stdout);

        fgets(buf, 1024, stdin);
        fprintf(stdout, "child Got: '%s'\n", buf);
        fflush(stdout);

        sleep(5);

        dbgMsg("noenoe\n");

        return 2;
    }

    int
    runParent() {
        FILE *cIn = childStdin();
        FILE *cOut = childStdout();
        FILE *cErr = childStderr();

        fprintf(cIn, ("1234567890\n"));

        bool doLoop = true;
        bool doWaitOut = true;
        bool doWaitErr = true;

        bool out, err;
        char buf[1024];
        (void)memset((void *)buf, 0, sizeof(buf));

        if (getProcessIPCType() != Process_IPC_Pipe) {
            doWaitErr = false;
        }

        while (doLoop == true &&
               (doWaitOut == true ||
                doWaitErr == true)) {
            dbgMsg("looping...\n");

            out = false;
            err = false;

            if ((doLoop = waitChildReadable(out, err)) == true) {

                if (doWaitOut == true && out == true) {
                    if (fgets(buf, 1024, cOut) != NULL) {
                        dbgMsg("Got stdout: '%s'\n", buf);
                    } else {
                        dbgMsg("stdout closed.\n");
                        doWaitOut = false;
                    }
                }
                if (doWaitErr == true && err == true) {
                    if (fgets(buf, 1024, cErr) != NULL) {
                        dbgMsg("Got stderr: '%s'\n", buf);
                    } else {
                        dbgMsg("stderr closed.\n");
                        doWaitErr = false;
                    }
                }
            }

        }
        return 0;
    }

public:
    MyProcess(void) :
        Process("", "", "", "") {
    }

    ~MyProcess(void) {
    }
};


#ifdef DO_COMPLETION
class Waker: public Thread {
private:
    Completion &mC;
    int mSec;

protected:
    int
    run(void) {
        dbgMsg("Enter, wait %d sec.\n", mSec);
        ::sleep(mSec);

        mC.wakeAll();
        dbgMsg("woke the sleeper up.\n");

        return 0;
    }

public:
    Waker(Completion &c, int sec) :
        mC(c),
        mSec(sec) {
    }
};
#endif // DO_COMPLETION


static void
setupSignals() {
    signal(SIGPIPE, SIG_IGN);

    SignalThread *st = new SignalThread();
    st->start(false, false);
}


class myArgs: public CommandLineParameters {
private:
    int mA;
    double mB;
    bool mC;
    const char *mD;
    bool mE;


private:

    static bool parseA(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        bool ret = parseInt32(arg, m->mA);
        if (ret == true) {
            dbgMsg("Called, got '%d'.\n", m->mA);
        }
        return ret;
    }
    friend bool parseA(const char *arg, void *ctx);

    static bool parseB(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        bool ret = parseDouble(arg, m->mB);
        if (ret == true) {
            dbgMsg("Called, got '%f'.\n", m->mB);
        }
        return ret;
    }
    friend bool parseB(const char *arg, void *ctx);

    static bool parseC(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        bool ret = parseBool(arg, m->mC);
        if (ret == true) {
            dbgMsg("Called, got '%s'.\n", booltostr(m->mC));
        }
        return ret;
    }
    friend bool parseC(const char *arg, void *ctx);

    static bool parseD(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        m->mD = (const char *)strdup(arg);
        dbgMsg("Called, got '%s'.\n", m->mD);
        return true;
    }
    friend bool parseD(const char *arg, void *ctx);

    static bool parseE(const char *arg, void *ctx) {
        (void)arg;
        myArgs *m = (myArgs *)ctx;
        m->mE = true;
        dbgMsg("Called, got '%s'.\n", booltostr(m->mE));
        return true;
    }
    friend bool parseE(const char *arg, void *ctx);

    static bool doUsage(const char *arg, void *ctx) {
        (void)arg;
        myArgs *m = (myArgs *)ctx;
        m->usage(true);
        return false;
    }


public:
    myArgs(const char *argv0) :
        CommandLineParameters(argv0) {

        addParser("--int-a", parseA, (void *)this,
                  Parse_Next, Value_Integer,
                  true, false, "parseA...");
        addParser("--int-a=", parseA, (void *)this,
                  Parse_ThisWithValue, Value_Integer,
                  true, false, "parseA...");

        addParser("--float-b", parseB, (void *)this,
                  Parse_Next, Value_Float,
                  true, false, "parseB...");
        addParser("--float-b=", parseB, (void *)this,
                  Parse_ThisWithValue, Value_Float,
                  true, false, "parseB...");

        addParser("--bool-c", parseC, (void *)this,
                  Parse_Next, Value_Boolean,
                  true, false, "parseC...");
        addParser("--bool-c=", parseC, (void *)this,
                  Parse_ThisWithValue, Value_Boolean,
                  true, false, "parseC...");

        addParser("--string-d", parseD, (void *)this,
                  Parse_Next, Value_String,
                  true, false, "parseD...");
        addParser("--string-d=", parseC, (void *)this,
                  Parse_ThisWithValue, Value_String,
                  true, false, "parseD...");

        addParser("--e", parseE, (void *)this,
                  Parse_This, Value_Boolean,
                  true, false, "parseE...");

        addParser("--?", doUsage, (void *)this,
                  Parse_This, Value_Boolean,
                  false, false, "Show this usage.");
        addParser("--help", doUsage, (void *)this,
                  Parse_This, Value_Boolean,
                  false, false, "Show this usage.");
    }
};


int
main(int argc, char *argv[]) {
    int newArgc;
    const char * const *newArgv;

    myArgs args(argv[0]);

    args.parse(argc - 1, argv + 1);

    args.getUnparsedOptions(newArgc, newArgv);
    for (int i = 0; i < newArgc; i++) {
        fprintf(stderr, "argv[%d]: '%s'\n", i, newArgv[i]);
    }

    dbgMsg("Enter.\n");

    setupSignals();
    ProcessJanitor::initialize();

    testBuffer x(10);

#ifdef DO_COMPLETION
    Mutex l;

    l.lock();
    Completion c(l);

    Waker t(c, 10);
    t.start();
    dbgMsg("Im about to sleep.\n");
    c.wait();
    dbgMsg("Woke up.\n");

#endif // DO_COMPLETION

    MyProcess p;
    p.start(Process::Process_Sync_Asynchronous,
            Process::Process_IPC_Pty);

    ::sleep(3);

    p.wait();
    fprintf(stderr, "%d\n", p.getExitCode());

    dbgMsg("Leave.\n");

    return 0;
}

