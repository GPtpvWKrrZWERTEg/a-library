#include <nata/nata_rcsid.h>


__rcsId("$Id: ttest3.cpp 83 2012-04-04 22:13:08Z mzbjyza2mji4zgz $")


#include <nata/nata_includes.h>
#include <nata/ChunkedBuffer.h>

#include <nata/SimpleFileParser.h>


class testConfig: public SimpleFileParser {
};


class testChunk: public ChunkedBuffer<uint16_t> {
private:
    static inline uint16_t *
    allocator(size_t s, void *dummy) {
        (void)dummy;
        dbgMsg("Called.\n");
        return (uint16_t *)malloc(s * sizeof(uint16_t));
    }

    static inline void
    destructor(uint16_t *ptr, void *dummy) {
        (void)dummy;
        dbgMsg("Called.\n");
        (void)free((void *)ptr);
    }


public:
    inline
    testChunk(void) {
        setAllocationHook(allocator, this);
        setDestructionHook(destructor, this);
    }
};


uint16_t u0[] = {
    0, 1, 2, 3
};

uint16_t u1[] = {
    4, 5, 6, 7
};

uint16_t u2[] = {
    8, 9, 10, 11
};


int
main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    nata_InitializeLogger(emit_Syslog, "hoehoe", false, false, 1);

    testChunk tc;

    size_t n;
    size_t i;
    uint16_t buf[32];
    size_t s;
    uint16_t u;

    tc.put(u0, 4);
    tc.put(u1, 4);

    n = tc.size();
    for (i = 0; i < n; i++) {
        if (tc.get(u) != true) {
            fatal("???\n");
        }
        fprintf(stderr, PFSzs(2, u) ": %u\n", i, u);
    }

    tc.put(u0, 4);
    tc.put(u1, 4);
    tc.put(u2, 4);

    s = 6;
    tc.get(buf, s);
    for (i = 0; i < s; i++) {
        fprintf(stderr, PFSzs(2, u) ": %u\n", i, buf[i]);
    }

    s = 6;
    tc.get(buf, s);
    for (i = 0; i < s; i++) {
        fprintf(stderr, PFSzs(2, u) ": %u\n", i, buf[i]);
    }

    return 0;
}
