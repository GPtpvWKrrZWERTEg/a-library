/*
 * $Id: nata_rcsid.h 99 2013-01-25 17:09:27Z mzbjyza2mji4zgz $
 */
#ifndef __NATA_RCSID_H__
#define __NATA_RCSID_H__

#ifdef __GNUC__
#define USE_BOODOO	__attribute__((used))
#else
#define USE_BOODOO	/**/
#endif /* __GNUC__ */

#define __rcsId(id) \
static volatile const char USE_BOODOO *rcsid(void) { \
    return (id != 0) ? id : rcsid(); \
}

#endif /* __NATA_RCSID_H__ */

