#ifndef CVM__ERRORS__H__
#define CVM__ERRORS__H__

#define CVME_GENERAL 1
#define CVME_BAD_CLIDATA 2
#define CVME_BAD_MODDATA 3
#define CVME_IO 4
#define CVME_NOFACT 5
#define CVME_CONFIG 6
#define CVME_NOCRED 7
#define CVME_PERMFAIL 100

/* This error code is only used by modules, to signal that the error
   is fatal and should cause module shutdown. */
#define CVME_FATAL 0x100

#define CVME_MASK  0x0ff

#endif
