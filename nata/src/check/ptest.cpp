#include <nata/nata_rcsid.h>
__rcsId("$Id: ptest.cpp 99 2013-01-25 17:09:27Z mzbjyza2mji4zgz $")

#include <nata/libnata.h>

#include <nata/Process.h>
#include <nata/ProcessJanitor.h>

#include <nata/Thread.h>
#include <nata/SignalThread.h>
#include <nata/SynchronizedMap.h>
#include <nata/Completion.h>

#include <nata/IOPipers.h>
#include <nata/FullSimplexedCommand.h>

#include <nata/nata_perror.h>

static void
ttest(void) {
    IOPipers ips;
    const char * const argv[] = { "cat", "/etc/hosts", NULL };

    FullSimplexedCommand p(NULL, "/bin/cat", (char * const *)argv);
    p.start();

    ips.addPiper(p.childOutFd(), 1);
    ips.addPiper(p.childErrFd(), 2);

    ips.start();

    ips.wait();
    p.wait();
}


class MyProcess: public Process {
protected:
    int
    runChild() {
        Process::unblockAllSignals();
        nata_InitializeLogger(emit_Unknown, "", true, false, 1);

        char buf[1024];

        dbgMsg("child start.\n");

        fprintf(stdout, "child stdout: I'm child, %d.\n", fileno(stdout));
        fflush(stdout);
        fprintf(stderr, "child stderr: I'm child, %d.\n", fileno(stderr));
        fflush(stderr);

        (void)fgets(buf, sizeof(buf), stdin);
        fprintf(stdout, "child Got: '%s'\n", buf);
        fflush(stdout);

        sleep(5);

        dbgMsg("child done.\n");

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


class Waker: public Thread {
private:
    Completion &mC;
    unsigned int mSec;

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
    Waker(Completion &c, unsigned int sec) :
        mC(c),
        mSec(sec) {
    }
};


static void
finalize(int sig) {
    (void)sig;
    dbgMsg("Exit.\n");
    exit(0);
}


static void
setupSignals() {
    SignalThread *st = new SignalThread();

    st->ignore(SIGPIPE);
    st->setHandler(SIGINT, finalize);
    st->start(false, false);
}


int
main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    nata_InitializeLogger(emit_Unknown, "", true, false, 1);

    dbgMsg("Enter.\n");

    setupSignals();
    ProcessJanitor::initialize();

#if 0
    Mutex l;

    l.lock();
    Completion c(&l);

    Waker t(c, 10);
    t.start();
    dbgMsg("Im about to sleep.\n");
    c.wait();
    dbgMsg("Woke up.\n");
#endif

    ttest();

    MyProcess p;


    if (strcmp(argv[1], "1") == 0) {
        p.start(Process::Process_Sync_Asynchronous,
                Process::Process_IPC_Pipe);
        fprintf(stderr, "child pid = %d.\n", p.getChildPid());
        p.wait();
        fprintf(stderr, "process exit %d.\n", p.getExitCode());
    }

    if (strcmp(argv[1], "2") == 0) {
        p.start(Process::Process_Sync_Synchronous,
                Process::Process_IPC_Pipe);
        fprintf(stderr, "child pid = %d.\n", p.getChildPid());
        p.wait();
        fprintf(stderr, "process exit %d.\n", p.getExitCode());
    }

    if (strcmp(argv[1], "3") == 0) {
        p.start(Process::Process_Sync_Asynchronous,
                Process::Process_IPC_Pty);
        fprintf(stderr, "child pid = %d.\n", p.getChildPid());
        p.wait();
        fprintf(stderr, "process exit %d.\n", p.getExitCode());
    }

    if (strcmp(argv[1], "4") == 0) {
        p.start(Process::Process_Sync_Synchronous,
                Process::Process_IPC_Pty);
        fprintf(stderr, "child pid = %d.\n", p.getChildPid());
        p.wait();
        fprintf(stderr, "process exit %d.\n", p.getExitCode());
    }

    dbgMsg("Leave.\n");

    return 0;
}

