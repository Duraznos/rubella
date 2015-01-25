// File: mumps/include/mumps.h
//
// module MUMPS header file - standard includes

/*      Copyright (c) 1999 - 2014
 *      Raymond Douglas Newman.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Raymond Douglas Newman nor the names of the
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// sizeof() for structures added 23 Sep 2013.  NOTE: will change with changes
// to constant defs and the sizeof() info below will not be updated - rdn

#ifndef _MUMPS_MUMPS_H_                         // only do this once
#define _MUMPS_MUMPS_H_

//** general constant definitions ***/

#define FALSE           0                       // nicer than using 0
#define TRUE            1                       // or 1

#define MUMPS_MAGIC     4155766917UL            // seems unique
#define MUMPS_SYSTEM    50                      // MDC assigned number
#define MAX_DATABASE_BLKS 2147483647            // max of 2**31-1 for now
#define VERSION_MAJOR   1                       // Major version number
#define VERSION_MINOR   64                      // Minor version number
#define VERSION_TEST	0                       // Test version number
#define MBYTE           1048576                 // 1024*1024
#define DAEMONS         10                      // Jobs per daemon
#define MIN_DAEMONS     2                       // minimum of these
#define MAX_DAEMONS     10                      // maximum of these
#define STORAGE         1024                    // what $STORAGE returns
#ifdef  __APPLE__
#define PRVGRP          80                      // admin in OSX
#else
#define PRVGRP          0                       // Priv group FreeBSD and linux
#endif                                          // darwin

#define MAX_INT_DIGITS	9                       // can be an int at 9
#define DEFAULT_PREC    12                      // number of decimal places
#define MAX_NUM_BYTES	63                      // max size of a number
#define MAX_STR_LEN     32767                   // max size of a string
#define MAX_ECODE       1024                    // max len for $ECODE

#define SECDAY          86400                   // seconds per day ($H)
#define YRADJ           47117                   // days from 1 Jan 1841 to 1970

#define UCIS            64                      // always 64

// KeyCmp outputs
#define KEQUAL           0                      // outputs for..
#define K2_LESSER        1                      // .. KeyCmp function
#define K2_GREATER      -1                      // ***

#define MAX_DO_FRAMES   128                     // maximum permitted do_frame
#define STM1_FRAME	MAX_DO_FRAMES-1             // where $ST(-1) data goes
#define MAX_SEQ_IO      16                      // maximum sequential io chans
#define MAX_SEQ_NAME	256                     // max file name size
#define MAX_SEQ_OUT     6                       // max output terminator size
#define MAX_DKEY_LEN	16                      // max $KEY seq stored
#define SQ_FREE		0                       // SQ_Chan->type - free
#define SQ_FILE		1                       // SQ_Chan->type - disk file
#define SQ_TCP		2                       // SQ_Chan->type - tcpip
#define SQ_PIPE		3                       // SQ_Chan->type - local pipe
#define SQ_TERM		4                       // SQ_Chan->type - device
#define SQ_LF		-1                      // WRITE !
#define SQ_FF		-2                      // WRITE #

#define SQ_USE_ECHO     1                       // turn echo on
#define SQ_USE_NOECHO	2                       // turn echo off
#define SQ_USE_ESCAPE	4                       // turn escape on
#define SQ_USE_NOESCAPE	16                      // turn escape off
#define SQ_USE_DISCON	128                     // disconnect client from sock
#define SQ_USE_DELNONE	256                     // no delete function
#define SQ_USE_DEL8     512                     // use backspace as delete
#define SQ_USE_DEL127	1024                    // use delete as delete
#define SQ_USE_DELBOTH	2048                    // use both as delete
#define SQ_CONTROLC     4096                    // enable control c trapping
#define SQ_NOCONTROLC	8192                    // no control c trap, ignore it
#define SQ_CONTROLT     16384                   // enable control t status
#define SQ_NOCONTROLT	32768                   // disable control t status

#ifdef __linux__
#define SHMAT_SEED      (void *)0x1000000
#else
#define SHMAT_SEED      (void *)0
#endif

#define MIN_GBD		40			// minumum number GBDs

// Note the following three MUST be a power of 2 as they are masks for &
#define GBD_HASH        1024                    // hash size for global buffers
#define NUM_DIRTY       1024                    // max queued dirty chains
#define NUM_GARB        8192                    // max queued garbage blocks

#define RBD_HASH        1023                    // hash size for routine names
#define GBD_FREE        GBD_HASH                // head of GBD free list

#define AVROUSIZ        3072                    // average compiled routine size
#define MAXROUSIZ       32767                   // max compiled rou size
#define MAXROULIN       32767                   // max rou lines
#define COMP_VER        7                       // compiler version
#define DB_VER          1                       // database version

// Global flags (from Global Directory) follow
#define GL_JOURNAL      1                       // Journal global flag
#define GL_TOP_DEFINED	2                       // Top node of global defined

#define LOCKTAB_SIZE    1024                    // 1kb per job
#define UCI_IS_LOCALVAR	255                     // for struct mvar
#define VAR_UNDEFINED	-1                      // undefined variable

#define MAX_ASTK        256                     // max depth of astk
#define MAX_SSTK        MBYTE                   // max string stack (1 MB)
#define MAX_ISTK        32768                   // max indirect stack

// Do frame types - negative numbers are error codes
#define TYPE_RUN        1                       // normal mumps startup
#define TYPE_JOB        2                       // got jobbed [0] only
#define TYPE_DO         3                       // DO
#define TYPE_EXTRINSIC	4                       // Extrinsic
#define TYPE_XECUTE     5                       // execute

#define DO_FLAG_TEST	1                       // $TEST value (0/1)
#define DO_FLAG_ATT     2                       // sym attach done
#define DO_FLAG_FOR     4                       // called from a FOR (infor)
#define DO_FLAG_ERROR	8                       // this is an error frame

// Signals we do something with (see jobtab->trap).  (Add as required)
#define SIG_HUP         1                       // SIGHUP	(ERR Z66)
#define SIG_CC          (1 << 2)                // control c signal (sigint)
#define SIG_QUIT        (1 << 3)                // SIGQUIT	(HALT)
#define SIG_TERM        (1 << 15)               // SIGTERM	(HALT)
#define SIG_STOP        (1 << 17)               // SIGSTOP	(HALT)
#define SIG_WS          (1 << 28)               // window size changes (ignore)
#define SIG_CT          (1 << 29)               // control t signal (siginfo)
#define SIG_U1          (1 << 30)               // user signal 1 (ERR Z67)
#define SIG_U2          (1 << 31)               // user signal 2 (ERR Z68)
// Unknown signals generate error Z69

#define MAX_VOL             2                   // max number of vols
#define VOL_FILENAME_MAX	256             // max chars in stored filename
#define JNL_FILENAME_MAX	226             // max chars in journal filenam

// systab->historic bit flag meanings
#define	HISTORIC_EOK		1               // E syntax flag
#define HISTORIC_OFFOK		2               // GO/DO with offset OK
#define HISTORIC_DNOK		4               // $NEXT OK runtime/runtime_ssvn

// Semaphore defines
// Semaphores are setup with a value equal to systab->maxjob
// A read takes one semaphore unit
// A write takes systab->maxjob units

#define SEM_SYS         0                       // Systab Semaphore
#define SEM_LOCK        1                       // Lock Table Semaphore
#define SEM_GLOBAL      2                       // global database module
#define SEM_ROU         3                       // routine buffers
#define SEM_WD          4                       // write daemons

#define SEM_MAX         5                       // total number of these

#if defined(__sun__) || defined(__NetBSD__)
union semun {
        int             val;            /* value for SETVAL */
        struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
        unsigned short  *array;         /* array for GETALL & SETALL */
};
typedef union semun semun_t;

#endif

#define MAX_TRANTAB	8                       // total number of entries

typedef unsigned long long      u_int64;        // unix unsigned quadword

typedef u_int64 chr_q;                          // our quadword special

typedef union __attribute__ ((__packed__)) VAR_U // get at this two ways
{ chr_q var_qu;                                 // variable name (quadword)
  u_char var_cu[8];                             // variable name (as char[])
} var_u;                                        // variable name union

typedef struct __attribute__ ((__packed__)) CSTRING // our string type
{ short len;                                    // length of it
  u_char buf[32768];                            // and the content
} cstring;                                      // end counted string

typedef struct __attribute__ ((__packed__)) MVAR // subscripted MUMPS var
{ var_u name;                                   // variable name
  u_char volset;                                // volset number
  u_char uci;                                   // uci# -> 255 = local var
  u_char slen;                                  // subs (key) length
  u_char key[256];                              // the subs (key) - allow for 0
} mvar;                                         // end MUMPS subs var
						// sizeof(mvar) = 267

//** common memory structures ***/

typedef struct __attribute__ ((__packed__)) UCI_TAB
{ var_u name;                                   // uci name
  u_int global;                                 // ptr to global directory
} uci_tab;                                      // define the uci table

typedef union __attribute__ ((__packed__)) DATA_UNION // diff types of msg data
{ struct GBD *gbddata;                          // a gbd pointer
  u_int intdata;                                // or an integer (block number)
} msg_data;                                     // end data msg union

typedef struct __attribute__ ((__packed__)) WD_TAB // write daemon table
{ int pid;                                      // the wd's pid
  int doing;                                    // what we are doing
  msg_data currmsg;                             // the current gbd */block#
} wdtab_struct;                                 // end write daemon structure

typedef struct __attribute__ ((__packed__)) LABEL_BLOCK
{ u_int magic;                                  // mumps magic number
  u_int max_block;                              // maximum block number
  int header_bytes;                             // bytes in label/map
  int block_size;                               // bytes per data block
  var_u volnam;                                 // volume name (8 bytes)
  short db_ver;                                 // database version
  u_char journal_available;                     // jrnl turned on at startup
  u_char journal_requested;                     // && journal_available = ON
  u_char clean;                                 // clean dismount flag
  char journal_file[JNL_FILENAME_MAX + 1];      // journal file name
  uci_tab uci[UCIS];                            // current ucis (at 256!!!)
} label_block;         				// define the label block
						// sizeof(label_block) = 1024

typedef struct __attribute__ ((__packed__)) DB_STAT
{ u_int dbget;                                  // Global Gets
  u_int dbset;                                  // Global Sets
  u_int dbkil;                                  // Global Kills
  u_int dbdat;                                  // Global $DATAs
  u_int dbord;                                  // Global $ORDERs
  u_int dbqry;                                  // Global $QUERYs

  u_int lasttry;                                // Search Last Tries
  u_int lastok;                                 // Search Last Successes

  u_int logrd;                                  // Logical Block Reads
  u_int phyrd;                                  // Physical Block Reads

  u_int logwt;                                  // Logical Block Writes
  u_int phywt;                                  // Physical Block Writes

  u_int blkalloc;                               // Block Allocates
  u_int blkdeall;                               // Block Deallocates
  u_int blkreorg;                               // Block Reorganizes
  u_int diskerrors;                             // Disk write errors
} db_stat;                                      // database statistics

struct GBD;                                     // defined in "db_util.h"
struct RBD;                                     // see compile.h

typedef struct __attribute__ ((__packed__)) VOL_DEF
{ label_block *vollab;                          // ptr to volset label block
  void *map;                                    // start of map area
  void *first_free;                             // first word with free bits
  struct GBD *gbd_hash[GBD_HASH+1];             // gbd hash table
  struct GBD *gbd_head;                         // head of global buffer desc
  int num_gbd;                                  // number of global buffers
  void *global_buf;                             // start of global buffers
  void *zero_block;                             // empty block in memory
  struct RBD *rbd_hash[RBD_HASH+1];             // head of routine buffer desc
  void *rbd_head;                               // head of routine buffer desc
  void *rbd_end;                                // first addr past routine area
  int num_of_daemons;                           // number of daemons
  wdtab_struct wd_tab[MAX_DAEMONS];             // write daemon info table
  int dismount_flag;                            // flag to indicate dismounting
  int map_dirty_flag;                           // set if map is dirty
  int writelock;                                // MUMPS write lock
  u_int upto;                                   // validating map up-to block
  int shm_id;                                   // GBD share mem id
  struct GBD *dirtyQ[NUM_DIRTY];                // dirty que (for daemons)
  int dirtyQw;                                  // write ptr for dirty que
  int dirtyQr;                                  // read ptr for dirty que
  u_int garbQ[NUM_GARB];                        // garbage que (for daemons)
  int garbQw;                                   // write ptr for garbage que
  int garbQr;                                   // read ptr for garbage que
  off_t jrn_next;                               // next free offset in jrn file
  char file_name[VOL_FILENAME_MAX];             // absolute pathname of volfile
  db_stat stats;                                // database statistics
} vol_def;                                      // end of volume def
						// sizeof(vol_def) = 57948

typedef struct __attribute__ ((__packed__)) DO_FRAME
{ u_char *routine;                              // addr of rou (or X src)
  u_char *pc;                                   // current mumps pc
  short *symbol;                                // process space sym ptrs
  u_char *newtab;                               // process space new table
  u_char *endlin;                               // address of current ENDLIN
  var_u rounam;                                 // routine name
  u_char vol;                                   // rou source vol set #
  u_char uci;                                   // rou source uci #
  u_short line_num;                             // current routine line#
  u_char estack;                                // current estack offset
  u_char type;                                  // see TYPE_??? def
  u_char level;                                 // current argless do level
  u_char flags;                                 // flags for this frame
  long savasp;                                  // saved asp
  long savssp;                                  // saved ssp
  long asp;                                     // entry asp
  long ssp;                                     // entry ssp
  long isp;                                     // entry indirect pointer
} do_frame;            				// do frame
						// sizeof(do_frame) = 96

// *** SEQIO specific *** //

typedef struct __attribute__ ((__packed__)) FORKTAB
{ int job_no;
  int pid;
} forktab;

typedef struct __attribute__ ((__packed__)) SERVERTAB
{ int slots;
  int taken;
  int cid;
  u_char name[MAX_SEQ_NAME];
  forktab *forked;
} servertab;

typedef struct __attribute__ ((__packed__)) SQ_CHAN
{ u_char type;                                  // type of device
  u_char options;                               // type specific options
  u_char mode;                                  // how object is opened
  int fid;                                      // os supplied file id

  servertab s;

  u_short dx;                                   // $X
  u_short dy;                                   // $Y
  u_char name[MAX_SEQ_NAME];                    // name of what was opened
  short dkey_len;                               // $KEY length stored
  u_char dkey[MAX_DKEY_LEN+1];                  // stored $KEY (null term)
  short out_len;                                // length of output terminator
  u_char out_term[MAX_SEQ_OUT];                 // the output terminator
  int in_term;                                  // input terminator bit mask
  var_u namespace;                              // routine for namespace
} SQ_Chan;                                      // define the $I stuf

// *** End SEQIO specific *** //

typedef struct __attribute__ ((__packed__)) JOBTAB
{ int pid;                                      // O/S PID (0 if unused)
  int cur_do;	                             	// current do frame addr
  u_int commands;                               // commands executed
  u_int grefs;                                  // global references
  u_int last_block_flags;                       // journal etc of last db block
  short error_frame;                            // frame error happened in
  short etrap_at;                               // where $ET was invoked
  int trap;                                     // outstanding traps
  int attention;                                // do something
  short async_error;                            // async erors
  short user;                                   // user number
  short priv;                                   // privs this job
  short precision;                              // decimal precision
  u_char io;                                    // current io index
  u_char test;                                  // current $TEST (0/1)
  u_char uci;                                   // current uci number
  u_char vol;                                   // current volset number
  u_char luci;                                  // current lock uci number
  u_char lvol;                                  // current lock volset number
  u_char ruci;                                  // current rou uci number
  u_char rvol;                                  // current rou volset number
  mvar last_ref;                                // $REFERENCE
  short start_len;                              // length start data
  u_char start_dh[14];                          // store start time here
  do_frame dostk[MAX_DO_FRAMES];                // the do stack
  SQ_Chan seqio[MAX_SEQ_IO];                    // sequential io stuf
  struct GBD *view[MAX_VOL];                    // locked view buffers
} jobtab;              				// define jobtab
						// sizeof(jobtab) = 21939

typedef struct __attribute__ ((__packed__)) LOCKTAB // internal lock tables
{ struct LOCKTAB *fwd_link;                     // point at next one
  int size;                                     // how many bytes
  short job;                                    // int job (-1 = free)
  short lock_count;                             // how many times locked by job
  short byte_count;                             // size of following reference
  u_char vol;                                   // vol number
  u_char uci;                                   // uci number (255 = local)
  var_u name;                                   // var name
  u_char key[256];                              // and the key
} locktab;             				// define locktab

typedef struct __attribute__ ((__packed__)) TRANTAB // translation table
{ var_u  from_global;                           // from global
  u_char from_vol;                              //      volumeset#
  u_char from_uci;                              //      uci#
  var_u  to_global;                             //   to global
  u_char to_vol;                                //      volumeset#
  u_char to_uci;                                //      uci#
} trantab;             				// define trantab

typedef struct __attribute__ ((__packed__)) SYSTAB // system tables
{ void *address;
  jobtab *jobtab;                               // address of jobtab
  int maxjob;                                   // maximum jobs permitted
  int sem_id;                                   // GBD semaphore id
  int historic;                                 // Enn, tag+off, $NEXT etc
  int precision;                                // decimal precision
  int max_tt;                                   // max TRANTAB used
  trantab tt[MAX_TRANTAB];                      // translation tables
  int start_user;                               // he's priv too
  void *lockstart;                              // head of lock table
  int locksize;                                 // how many bytes
  locktab *lockhead;                            // head of used locks
  locktab *lockfree;                            // head of lock free space
  long addoff;                                  // off from systab to add buff
  long addsize;                                 // add buff size
  vol_def *vol[MAX_VOL];                        // array of vol ptrs
  u_int last_blk_used[1];                       // actually setup for real jobs
} systab_struct;                                // end of systab
                                                // Followed by jobtab.
						// sizeof(systab_struct) = 256

extern systab_struct *systab;                   // make its ptr external
extern int sem_id;                              // global semaphore id

//** process memory structures ***/
//** PARTAB definitions **

typedef struct __attribute__ ((__packed__)) PARTAB // define the partition table
{ jobtab *jobtab;                               // our jobtab entry
  int vol_fds[MAX_VOL];                         // the filedes for the volumes
  int jnl_fds[MAX_VOL];                         // the filedes for journals
  int debug;                                    // debug in progress
  u_char *sstk_start;                           // start of string stack
  u_char *sstk_last;                            // last byte of sstk
  var_u *varlst;                                // var list for compiler
  int checkonly;                                // used by compiler
  u_char **sp;                                  // source ptr for compile
  cstring **lp;                                 // start of the line (ditto)
  int *ln;                                      // line num for $&%ROUCHK()
  mvar src_var;                                 // temp space for src mvar
}partab_struct;        				// end of partab type
						// sizeof(partab) = 339

extern partab_struct partab;                    // globalize partab
extern u_char *astk[];                          // address stack
extern u_char sstk[];                           // string stack
extern u_char *mumpspc;                         // mumps prog pointer


#endif                                          // !_MUMPS_MUMPS_H_
