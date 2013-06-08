/*
 * $Id: BlockingContainer.h 16 2011-07-14 22:46:22Z mzbjyza2mji4zgz $
 */
#ifndef __BLOCKINGCONTAINER_H__
#define __BLOCKINGCONTAINER_H__


#include <nata/nata_rcsid.h>


namespace BlockingContainerStatics {
    static const char *errorString[] = {
        "No error.",
        "Timedout.",
        "Container is no longer valid.",
        "Any failure.",
        NULL
    };
}


class BlockingContainer {


private:
    __rcsId("$Id: BlockingContainer.h 16 2011-07-14 22:46:22Z mzbjyza2mji4zgz $");


public:
    BlockingContainer(void) {
        (void)rcsid();
    }


    virtual
    ~BlockingContainer(void) {
    }


    typedef enum {
        Status_OK = 0,
        Status_Timedout,
        Status_Container_No_Longer_Valid,
        Status_Any_Failure
    } ContainerStatus;


    static const char *
    errorString(ContainerStatus e) {
        return BlockingContainerStatics::errorString[(int)e];
    }


    static bool
    isContinuable(ContainerStatus e) {
        return ((int)e <= (int)Status_Timedout) ? true : false;
    }
};


#endif // ! __BLOCKINGCONTAINER_H__
