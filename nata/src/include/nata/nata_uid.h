/* 
 * $Id: nata_uid.h 60 2011-12-14 21:28:38Z mzbjyza2mji4zgz $
 */
#ifndef __NATA_UID_H__
#define __NATA_UID_H__


typedef uint8_t nata_Uid[32];

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

extern void		nata_initUid(void);
extern void		nata_getUid(nata_Uid *uPtr);

#if defined(__cplusplus)
}
#endif /* __cplusplus */


#endif /* ! __NATA_UID_H__ */
