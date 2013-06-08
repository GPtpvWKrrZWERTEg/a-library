#include <nata/nata_rcsid.h>
__rcsId("$Id: ptest1.cpp 99 2013-01-25 17:09:27Z mzbjyza2mji4zgz $")

#include <nata/libnata.h>

#include <nata/Process.h>
#include <nata/ProcessJanitor.h>
#include <nata/SignalThread.h>

#include <nata/PipelinedCommands.h>
#include <nata/RegularExpression.h>
#include <nata/CommandLineParameters.h>

#include <vector>

#include <nata/nata_perror.h>

class myArgs: public CommandLineParameters {
private:
    typedef std::vector<const char *> mOptionList;

    mOptionList mCppOptions;
    mOptionList mLinkerOptions;


    char * const *
    mGetOptions(mOptionList &l) {
        char **ret = NULL;
        size_t n = l.size();
        if (n > 0) {
            ret = (char **)malloc((n + 1)* sizeof(char *));
            if (ret != NULL) {
                size_t i;
                for (i = 0; i < n; i++) {
                    ret[i] = (char *)l[i];
                }
                ret[n] = NULL;
            }
        }
        return (char * const *)ret;
    }


    static inline bool
    mParseAndAdd(myArgs *m, const char *arg, mOptionList &l) {
        bool ret = false;
        const char *cnm = strchr(arg, ',');

        if (cnm != NULL && *(cnm + 1) != '\0') {
            char *tokens[1024];
            char *args = nata_TrimRight((cnm + 1), " \t\r\n");
            int nTokens = nata_GetToken(args, tokens, 1024, ",");
            int i;
            for (i = 0; i < nTokens; i++) {
                l.push_back(strdup(tokens[i]));
            }
            (void)free((void *)args);
            ret = true;
        } else {
            fprintf(stderr, "%s: invalid option \"%s\".\n",
                    m->commandName(), arg);
        }

        return ret;
    }

    static bool parseSpecificCppOptions(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        m->mCppOptions.push_back(strdup(arg));
        return true;
    }
    friend bool parseSpecificCppOptions(const char *arg, void *ctx);

    static bool parseGenericCppOptions(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        return mParseAndAdd(m, arg, m->mCppOptions);
    }
    friend bool parseGenericCppOptions(const char *arg, void *ctx);

    static bool parseSpecificLinkerOptions(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        m->mLinkerOptions.push_back(strdup(arg));
        return true;
    }
    friend bool parseSpecificLinkerOptions(const char *arg, void *ctx);

    static bool parseGenericLinkerOptions(const char *arg, void *ctx) {
        myArgs *m = (myArgs *)ctx;
        return mParseAndAdd(m, arg, m->mLinkerOptions);
    }
    friend bool parseGenericLinkerOptions(const char *arg, void *ctx);


public:
    myArgs(const char *argv0) :
        CommandLineParameters(argv0) {

        addParser("-Wp,.*",
                  parseGenericCppOptions, (void *)this,
                  Parse_This, Value_String, true, true, NULL);
        addParser("-[DUI]..*",
                  parseSpecificCppOptions, (void *)this,
                  Parse_This, Value_String, true, true, NULL);

        addParser("-Wl,.*",
                  parseGenericLinkerOptions, (void *)this,
                  Parse_This, Value_String, true, true, NULL);
        addParser("-[Ll]..*",
                  parseSpecificLinkerOptions, (void *)this,
                  Parse_This, Value_String, true, true, NULL);

    }

    ~myArgs(void) {
        size_t i;

        for (i = 0; i < mCppOptions.size(); i++) {
            (void)free((void *)mCppOptions[i]);
        }
        for (i = 0; i < mLinkerOptions.size(); i++) {
            (void)free((void *)mLinkerOptions[i]);
        }
    }

    inline char * const *
    getCppOptions(void) {
        return mGetOptions(mCppOptions);
    }

    inline char * const *
    getLinkerOptions(void) {
        return mGetOptions(mLinkerOptions);
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
    nata_InitializeLogger(emit_Unknown, "", true, false, 0);

    dbgMsg("Enter.\n");

    setupSignals();
    ProcessJanitor::initialize();

    myArgs opts(argv[0]);
    if (opts.parse(argc - 1, argv + 1) != true) {
        return 1;
    }

    char * const *cppOpts = opts.getCppOptions();
    char * const *ldOpts = opts.getLinkerOptions();
    char * const *cp;
    const char * const *newArgv = NULL;
    int newArgc = 0;

    opts.getUnparsedOptions(newArgc, newArgv);

    if (cppOpts != NULL) {
        cp = cppOpts;
        fprintf(stderr, "CPPFLAGS = ");
        while (*cp != NULL) {
            fprintf(stderr, "%s ", *cp);
            cp++;
        }
        fprintf(stderr, "\n");
    }

    if (ldOpts != NULL) {
        cp = ldOpts;
        fprintf(stderr, "LDFLAGS = ");
        while (*cp != NULL) {
            fprintf(stderr, "%s ", *cp);
            cp++;
        }
        fprintf(stderr, "\n");
    }

    if (newArgc > 0) {
        int i;
        fprintf(stderr, "other = ");
        for (i = 0; i < newArgc; i++) {
            fprintf(stderr, "%s ", newArgv[i]);
        }
        fprintf(stderr, "\n");
    }

    return 0;

    const char * const cmd0[] = {
        "cat", "/etc/passwd", NULL
    };
    const char * const cmd1[] = {
        "sed", "s/^[a-zA-Z_][a-zA-Z0-9_\\-]*:/XXXXXXXX:/g", NULL
    };
    const char * const cmd2[] = {
        "cat", "-n", NULL
    };

    PipelinedCommands pc(-1, 1, 2);
    pc.addCommand(NULL, cmd0[0], (char * const *)cmd0);
    pc.addCommand(NULL, cmd1[0], (char * const *)cmd1);
    pc.addCommand(NULL, cmd2[0], (char * const *)cmd2);

    pc.start();

    return 0;
}
