/* 
 * $Id: nata_perror.h 60 2011-12-14 21:28:38Z mzbjyza2mji4zgz $
 */
#include <nata/nata_logger.h>

#ifdef perror
#undef perror
#endif /* perror */
#define perror(str)	nata_MsgError("%s: %s\n", str, strerror(errno))
