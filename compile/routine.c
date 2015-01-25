// File: mumps/compile/routine.c
//
// module compile - parse a routine ref

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


#include <stdio.h>                              // always include
#include <stdlib.h>                             // these two
#include <sys/types.h>                          // for u_char def
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>                              // error stuf
#include <limits.h>                     	// for LONG_MAX etc
#include <math.h>
#include "mumps.h"                              // standard includes
#include "proto.h"                              // standard prototypes
#include "error.h"                              // and the error defs
#include "opcodes.h"                            // and the opcodes
#include "compile.h"				// compile stuff

// function routine entered with source_ptr pointing at the source
// to evaluate and comp_ptr pointing at where to put the code.
//
// runtime is: 0 for compile, 1 for runtime and 2 for $TEXT() runtime
//	      -2 for $TEXT() compile
//
// Return       Means
// >0		reserved for byte offset
// 0 		Compiled indirect to end up at astk[asp]
// -1		Compiled just a tag (8 bytes)
// -2		Compiled routine and tag (16 bytes)
// -3		Compiled routine (no tag) (8 bytes)
// -4		Runtime only, routine, tag and offset (18 bytes)

short routine(int runtime)			// parse routine ref
{ char c;                                       // current character
  int i = 0;					// a usefull int
  int j;					// and another
  int dp = 0;					// dp in offset - blah
  var_u tag;                                    // to hold the tag
  var_u rou;                                    // to hold the rou
  int ntag;					// numeric tag flag
  int isinder = 0;				// indirect flag
  int offset = 0;				// tag offset
  int gotplus = 0;              // Flag for offset
  int p1indirect = 0;         // Piece one is indirected
  short s;
  tag.var_qu = 0;				// clear the tag
  rou.var_qu = 0;				// and the routine

  // If initial atom is an indirect string, evaluate it and put on sstk as a variable.

  c = *source_ptr++;				// get first character
  if (c == '@')					// if it's indirect
  { isinder = 1;				// say indirect
    atom();					// stack it
    p1indirect = 1;        // piece 1 is indirect... make sure to concat later.
    c = *source_ptr++;				// get the next
    if ((c == ')') ||				// check for end of it all
	(c == ',') ||
	(c == ' ') ||
	(c == '\0'))
    { source_ptr--;
      return 0;
    }
  }
    
  // If initial atom is a tag, extract first
    
  else
  { ntag = isdigit((int)c);			// check for a numeric tag
    for (i = 0; i < 8; i++)
    { if (ntag && (!isdigit((int)c))) break;	// numeric tag is all digits
      if ((i != 0) && (c == '%')) break;	// % only permitted as first
      if ((!isalnum((int)c)) && (c != '%')) break; // allow alphanumeric + %
      tag.var_cu[i] = c;			// save that one
      c = *source_ptr++;			// get the next
    }
    while (isalnum((int)c)) c = *source_ptr++;	// skip alpha numerics
  }

  // The following ONLY executes if we have a tag && $TEXT -->
  // append tag to stack as a string.
  if (runtime == -2)				// if $TEXT() compile
  { if ((!isinder) && (tag.var_qu != 0))       //if not indirected & tag is defined
    { *comp_ptr++ = OPSTR;			// string follows
      s = (short) i;
      bcopy(&s, comp_ptr, 2);
      comp_ptr += 2;
      //*((short *)comp_ptr)++ = (short) i;	// the size
      for (j = 0; j < i; *comp_ptr++ = tag.var_cu[j++]); // copy tag
      *comp_ptr++ = '\0';			// and null terminate
    }
    runtime = 0;				// pretend a compile
    isinder = 1;				// and now indirect
  }
  // <-- end execute only if tag

  if ((c == ')') ||				// check for end of it all
      (c == ',') ||
      (c == ' ') ||
      (c == '\0'))
  { source_ptr--;
    goto exit;
  }
    
  // The following is for a numeric offset -->

  if (c == '+')					// bloody offset
  { gotplus = 1;                // flag it
    if (!runtime)				// if just compiling
    { if ((!isinder) && (tag.var_qu != 0))
      { *comp_ptr++ = OPSTR;			// string follows
        s = (short) i;
        bcopy(&s, comp_ptr, 2);
        comp_ptr += 2;
        //*((short *)comp_ptr)++ = (short) i;	// the size
        for (j = 0; j < i; *comp_ptr++ = tag.var_cu[j++]); // copy tag
        *comp_ptr++ = '\0';			// and null terminate
      }
      isinder = 1;				// and now indirect

      *comp_ptr++ = OPSTR;			// string follows
      *comp_ptr++ = 1;              // string size of 1
      *comp_ptr++ = 0;              // pad byte for short variable
      //*((short *)comp_ptr)++ = (short) 1;	// the size
      *comp_ptr++ = '+';			// add the plus
      *comp_ptr++ = '\0';			// and null terminate
      if (tag.var_qu != 0 || p1indirect)	// if we have a tag or 1st piece is indirect
      { *comp_ptr++ = OPCAT;			// concatenate
      }
      eval();					// get the value
      *comp_ptr++ = OPPLUS;			// force evaluation
      *comp_ptr++ = OPCAT;			// concatenate
    }
    else					// it is runtime
    { if (!isdigit(*source_ptr))		// next must be a digit
      { comperror(-ERRM5);			// complain
        return -1;				// so caller won't change it
      }
      while (TRUE)				// while we have digits
      { if (*source_ptr == '.')			// if a bloody dot
        { if (dp)				// if already got one
          { break;				// just exit
          }
          dp = 1;				// flag it
          source_ptr++;				// ignore it
          continue;				// and continue
        }
        if (!isdigit(*source_ptr))
        { break;
        }
        if (!dp)				// if part of int
        { offset = (offset * 10) + *source_ptr++ - '0'; // convert it
        }
	else
	{ source_ptr++;				// just ignore it
	}
      }
      if (runtime == 1)				// if not $TEXT
      { if (!(systab->historic & HISTORIC_OFFOK)) // if offset not OK
	    { comperror(-(ERRMLAST+ERRZ70));	// complain
          return -1;				// so caller won't change it
        }
      }						// end runtime code
    }						// end offset code
    c = *source_ptr++;				// get the next
  }						// end offset junk
  // <-- end numeric offset
    
  if ((c == ')') ||				// check for end of it all
      (c == ',') ||
      (c == ' ') ||
      (c == '\0'))
  { source_ptr--;
    goto exit;
  }

  // The following resolves a routine name ^XYZ or ^@XYZ -->
    
  if (c == '^')					// if it's a routine
  { c = *source_ptr++;				// get next character
    if (c != '@')				// if not indirect
    { for (i = 0; i < 8; i++)       // increment i to be routine name length as follows
      { if ((i == 0) && (isdigit((int)c) != 0)) break; // can't start with a number
        if ((i != 0) && (c == '%')) break;	// % only permitted as first
        if ((isalnum((int)c) == 0) && (c != '%')) break; // allow alphanumeric + %
        rou.var_cu[i] = c;			// save that one
        c = *source_ptr++;			// get the next
      }
      while (isalnum((int)c) != 0) c = *source_ptr++; // skip alpha numerics
      if (isinder)				// is indirect already
      { *comp_ptr++ = OPSTR;			// string follows
        s = (short) (i + 1);            // Routine Name Length plus ^
        bcopy(&s, comp_ptr, 2);         // Store short int
        comp_ptr += 2;                  // Move past it
        //*((short *)comp_ptr)++ = (short) (i + 1); // the size
	    *comp_ptr++ = '^';			// store the caret
        for (j = 0; j < i; *comp_ptr++ = rou.var_cu[j++]); // copy rou
        *comp_ptr++ = '\0';			// and null terminate
        if ((tag.var_qu != 0) || (gotplus) || p1indirect)
        { *comp_ptr++ = OPCAT;			// concatenate
        }
      }
      source_ptr--;
    }

    else                                // indirect (^@XYZ)
    { 
      if ((!isinder) && (tag.var_qu != 0))  // if a $TEXT compile, this never executes b/c $T sets isinder to 1
                                            // Therefore, it's only for D TAG^@XYZ
        //SMH: NB: The code here is EXACTLY the same as for $Text (up above). We perhaps should merge them.
      { // *comp_ptr++ = OPCAT;			// concatenate previous --SMH - WRONG - NO PREVIOUS FOR SURE IF THIS IS A TAG!
        *comp_ptr++ = OPSTR;			// string follows
        s = (short) i;
        bcopy(&s, comp_ptr, 2);
        comp_ptr += 2;
        //*((short *)comp_ptr)++ = (short) i;	// the size
        for (j = 0; j < i; *comp_ptr++ = tag.var_cu[j++]); // copy tag
        *comp_ptr++ = '\0';			// and null terminate
      } 
      isinder = 1;				// and now indirect
      
      if (offset)
      { *comp_ptr++ = OPSTR;			// string follows
        *comp_ptr++ = 1;
        *comp_ptr++ = 0;
        //*((short *)comp_ptr)++ = (short) 1;	// the size
        *comp_ptr++ = '+';			// add the plus
        *comp_ptr++ = '\0';			// and null terminate
        if (tag.var_qu != 0)			// if we have a tag
        { *comp_ptr++ = OPCAT;			// concatenate
        }
        eval();					// get the value
        *comp_ptr++ = OPPLUS;			// force evaluation
        *comp_ptr++ = OPCAT;			// concatenate
      }

      *comp_ptr++ = OPSTR;			// string follows
      *comp_ptr++ = 1;
      *comp_ptr++ = 0;
      //*((short *)comp_ptr)++ = 1;		// the size
      *comp_ptr++ = '^';			// the caret
      *comp_ptr++ = '\0';			// and null terminate
      if ((tag.var_qu != 0) || (gotplus) || p1indirect)
      { *comp_ptr++ = OPCAT;			// concatenate it
      }
      atom();					// get routine name
      *comp_ptr++ = OPCAT;			// concatenate it
    }
  }						// end routineref
  else        // no ^ has been found.
  { source_ptr--;
  }

exit:
  if (isinder)					// if there was indirection
  { return 0;					// exit flagging that
  }

  // Below if we are doing a DO sans indirection... we must copy tag and rou to comp_ptr.
  if ((tag.var_qu != 0) || (offset))		// if we have a tag
  { ntag = 1;					// assume just a tag
    if ((rou.var_qu != 0) || (offset))		// if we have a routine
    { ntag = 2;					// say both
      bcopy(&rou.var_qu, comp_ptr, 8);
      comp_ptr += 8;
      //*((chr_q *)comp_ptr)++ = rou.var_qu;	// save the routine
    }
    bcopy(&tag.var_qu, comp_ptr, 8);
    comp_ptr += 8;
    //*((chr_q *)comp_ptr)++ = tag.var_qu;	// save the tag
    if (offset)					// if we have an offset
    { s = (short) offset;
      bcopy(&s, comp_ptr, 2);
      comp_ptr += 2;
      //*((short *)comp_ptr)++ = (short) offset;	// compile it in
      return -4;				// and exit
    }
    return -ntag;				// return saying 1 or 2
  }
  bcopy(&rou.var_qu, comp_ptr, 8);
  comp_ptr += 8;
  //*((chr_q *)comp_ptr)++ = rou.var_qu;		// save the routine
  return -3;					// say just the routine
}

// The following function compiles a routine, the result of code like:
//   MERGE ^$ROUTINE(routine_name) = local or global ref
// This always suceeds even if the routine is junk.
// The name is checked first tho...
//
// If rou == NULL, check the routine->src

short Compile_Routine(mvar *rou, mvar *src, u_char *stack)
{ cstring *line;				// the source line
  u_char *code;					// the code
  short s, ss;					// for returns
  int cnt;					// count things
  int nsubs = 0;				// count subscripts
  u_char src_slen;				// key in source
  u_char rou_slen = 0;				// key in routine
  u_char temp[100];				// temp space
  tags tag_tbl[256];				// space for the tags
  var_u var_tbl[256];				// and the variables
  var_u var;					// for one var
  int num_tags = 0;				// count tags
  int num_vars = 0;				// and variables
  int lino = 0;					// current line number
  int last_dots = 0;				// remember last line dots
  int i;					// a handy int
  int j;					// and another
  cstring *cptr;				// a cstring pointer
  u_char *p;					// and a char ptr
  var_u rounam;					// the routine name
  int same = 0;					// same routine flag

  partab.checkonly = 0;				// a real compile
  partab.ln = &lino;				// save for $&%ROUCHK()
  line = (cstring *) stack;			// for source lines
  code = stack + sizeof(cstring);		// where the code goes
  cptr = (cstring *) temp;			// point at temp space

  for (i = 0; i < 256; var_tbl[i++].var_qu = 0); // clear var table

  if ((code + MAXROUSIZ) > partab.sstk_last)	// too big ?
    return -(ERRMLAST+ERRZ8);			// yes - complain
  if (rou != NULL)
  { s = SS_Norm(rou);				// normalize mvar
    if (s < 0) return s;			// quit on error
  }
  i = 0;					// clear i
  if (rou != NULL)				// if it's real
  { while (i < rou->slen)			// for all subs
    { cnt = 0;					// flag no rabit ears
      if (nsubs > 0) return (-ERRM38);		// junk
      if (rou->slen > 20) return (-ERRM38);	// ditto
      s = UTIL_Key_Extract( &rou->key[i],	// key from here
			    temp,		// where to put it
			    &cnt);		// the count
      if (s < 0) return s;			// die on error
      if ((s > 8) || (s < 1))			// routine length (1 to 8)
        return (-ERRM38);			// junk
      i = i + cnt;                              // count used bytes
      nsubs++;					// count it
    }
    for (i = 0; i < 8; i++)			// scan the routine name
    { if ((i > 0) && !(temp[i]))		// done
      { for (j = i; j < 8; j++) rounam.var_cu[j] = '\0'; // copy 0
        break;					// and exit
      }
      if (isalpha(temp[i]))			// alpha ok anywhere
      { rounam.var_cu[i] = temp[i];		// copy it
        continue;				// and continue
      }
      if (isdigit(temp[i]) && (i))		// digit ok after first
      { rounam.var_cu[i] = temp[i];		// copy it
        continue;				// and continue
      }
      if ((temp[i] == '%') && !i)		// % ok as first
      { rounam.var_cu[i] = temp[i];		// copy it
        continue;				// and continue
      }
      return (-ERRM38);				// junk
    }						// destination now validated
  }						//
  else
  { partab.checkonly = 1;			// just a check
    same = 1;					// stop writting
    partab.sp = &source_ptr;			// where source ptr is
    partab.lp = &line;				// and where line is
  }
  if (src->name.var_cu[0] == '$')		// source an ssvn?
  { s = SS_Norm(src);				// normalize mvar
    if (s < 0) return s;			// quit on error
    if (bcmp(src->name.var_cu, "$ROUTINE", 8))	// a routine?
      return (-ERRM38);				// junk
    if (!partab.checkonly)			// if it's real
      if (rou->volset == src->volset)		// same volset
        if (rou->uci == src->uci)		// same uci
	  if (rou->slen == src->slen)		// same key size
	    if (bcmp(rou->key, src->key, rou->slen) == 0) // same key
	      same = 1;				// same rou and source
  }						// end source ssvn check
  if (!partab.checkonly)			// if it's a real compile
  { s = SemOp(SEM_ROU, -systab->maxjob);	// grab the routine semaphore
    if (s < 0) return s;			// if we got an error, quit
    rou_slen = rou->slen;			// save routine key size
  }
  if (!same)					// if not the same
  { s = DB_Kill(rou);				// dong it
    if (s < 0)					// complain on error
    { i = SemOp(SEM_ROU, systab->maxjob);	// release sem
      return s;					// exit
    }
    Routine_Delete(rounam.var_qu, rou->uci);	// delete the routine
  }
  src_slen = src->slen;				// save source key size

  line->buf[0] = '0';				// seed the $O()
  line->buf[1] = '\0';				// null terminated
  line->len = 1;				// this long
  s = UTIL_Key_Build(line, &src->key[src_slen]); // build the key
  src->slen = src_slen + s;			// store the new length
  comp_ptr = code;				// setup the compiler ptr
  partab.varlst = var_tbl;			// for localvar()
  while (TRUE)					// for all lines
  { 
    s = Dorder1(line->buf, src);		// get next in source
    if (!s) break;				// all done
    line->len = s;				// save length
    s = UTIL_Key_Build(line, &src->key[src_slen]); // build the key
    src->slen = src_slen + s;			// store the new length
    s = Dget1(line->buf, src);			// get the data
    if (s < 1) continue;			// ignore empty/undefined lines
    line->len = s;				// save the length
    source_ptr = line->buf;			// where the source is
    if (isalnum(*source_ptr) || (*source_ptr == '%')) // check for a tag
    { j = isdigit(*source_ptr);			// remember if digit
      if (num_tags == 255)			// check number of tags
      { comperror(-(ERRZ53+ERRMLAST));		// complain
        continue;				// ignore remainder of line
      }
      tag_tbl[num_tags].code = (comp_ptr - code); // save code offset
      tag_tbl[num_tags].name.var_qu = 0;	// zot the name
      tag_tbl[num_tags].name.var_cu[0] = *source_ptr++; // copy first char
      i = 1;					// init name index
      while (TRUE)
      { if (!isalnum(*source_ptr)) break;	// give up if wrong
	if (j && !isdigit(*source_ptr)) break;	// must be all digits
	if (i < 8)				// still copying ?
	  tag_tbl[num_tags].name.var_cu[i++] = *source_ptr; // copy one
	source_ptr++;				// increment source pointer
      }
      for (i = 0; i < num_tags; i++)		// check for duplicate
        if (tag_tbl[num_tags].name.var_qu ==
            tag_tbl[i].name.var_qu)		// the same ?
	  { comperror(-(ERRZ65+ERRMLAST));	// complain
	    p = comp_ptr;			// save
	    comp_ptr = code + tag_tbl[i].code;	// point at other one
	    comperror(-(ERRZ65+ERRMLAST));	// complain there too
	    comp_ptr = p;			// restore comp pointer
            continue;				// ignore remainder of line
	  }
      num_tags++;				// increment
      if (*source_ptr == '(')			// any argument list ?
      { cnt = 0;				// to count formal list
	source_ptr++;				// skip the (
	*comp_ptr++ = LOADARG;			// add the opcode
	p = comp_ptr;				// remember where the count is
	comp_ptr++;				// skip the count
	while (TRUE)				// scan the list
	{ if (*source_ptr == ')')		// found end yet?
	  { source_ptr++;			// skip )
	    break;				// and exit
	  }
	  var.var_qu = 0;			// clear var name
	  for (i = 0; i < 8; i++)		// scan possible var name
	  { if ((isalpha(*source_ptr)) ||
	        ((*source_ptr == '%') &&
	         (!i)))				// first char alpha or %
	    { var.var_cu[i] = *source_ptr++;	// copy it
	      continue;				// and go for more
	    }
	    if ((isalnum(*source_ptr)) && (i))	// the rest alpha/numeric
	    { var.var_cu[i] = *source_ptr++;	// copy it
	      continue;				// and go for more
	    }
	    if ((*source_ptr == ',') && (i))	// end of name
	      break;				// exit
	    if (*source_ptr == ')')		// end of name
	      break;				// exit
	    cnt = -(ERRMLAST+ERRZ13);		// else error
	    break;				// and exit
	  }
	  if (cnt < 0) break;			// quit on error
	  while (isalnum(*source_ptr)) source_ptr++; // skip long name
	  if (*source_ptr == ',')
	    source_ptr++;			// skip the comma
	  else if (*source_ptr != ')')		// check for )
	  { cnt = -(ERRMLAST+ERRZ13);		// error
	    break;				// exit
	  }
	  for (i = 0; i < 256; i++)		// scan var list
	  { if (var_tbl[i].var_qu == var.var_qu)
	      break;				// quit on match
	    if (var_tbl[i].var_qu == 0)
	    { var_tbl[i].var_qu = var.var_qu;	// save it
	      break;				// and exit
	    }
	  }
	  if (i == 256)				// too many?
	  { cnt = (-(ERRZ53+ERRMLAST));		// too many
	    break;				// exit
	  }
	  *comp_ptr++ = (u_char) i;		// save index
	  cnt++;				// count it
	  for (j = 1; j < cnt; j++)		// scan what's already there
	    if (p[j] == i)			// if already got that one
	    { cnt = -ERRM21;			// complain
	      break;				// exit
	    }
	}
	if (cnt > 255) cnt = (-(ERRZ53+ERRMLAST)); // too many
	if (cnt < 0)				// got an error?
	{ --p;					// back up the ptr
	  comp_ptr = p;				// backup
	  comperror(cnt);			// compile error
	  continue;				// ignore rest of line
	}
	*p = (u_char) cnt;			// save the count
      }
    }						// end tag processing
    lino++;					// count a line
    if (!same)					// write if reqd
    { for (i = 0; source_ptr[i] == '\t'; source_ptr[i++] = ' ');
						// convert leading tab to space
      cptr->len = itocstring(cptr->buf, lino);	// convert to a cstring
      s = UTIL_Key_Build(cptr, &rou->key[rou_slen]); // build the key
      rou->slen = rou_slen + s;			// store the new length
      s = DB_Set(rou, line);			// write out the source line
      if (s < 0)
      { if (!partab.checkonly) SemOp(SEM_ROU, systab->maxjob); // unlock
        partab.varlst = NULL;			// for localvar()
        return s;				// exit with error
      }
    }
    if (lino > MAXROULIN)
    { comperror(-(ERRZ54+ERRMLAST));		// complain
      continue;					// ignore the rest
    }
    if (*source_ptr == ';') continue;		// ignore comment lines
    if (*source_ptr++ != ' ')
    { comperror(-(ERRMLAST+ERRZ13));		// must be a space
      continue;					// ignore the rest
    }
    while (*source_ptr == ' ') source_ptr++;	// skip spare spaces

    *comp_ptr++ = LINENUM;			// mark new line
    s = (u_short) lino;
    bcopy(&s, comp_ptr, 2);
    comp_ptr += 2;
    //*((u_short *)comp_ptr)++ = (u_short) lino;	// and the line number
    p = comp_ptr;				// where the offset will go
    *comp_ptr++ = 0;				// these were
    *comp_ptr++ = 0;				// *((short *)comp_ptr)++ = 0
    j = 0;					// a dot counter
    if (*source_ptr == '.')			// any dots ?
    { for (i = 0; ; i++)			// scan them
      { if (*source_ptr == '.')			// found a dot
        { j++;					// count it
          source_ptr++;				// go past it
          continue;				// go for more
        }
        if (*source_ptr == ' ')			// a space?
        { source_ptr++;				// go past it
          continue;				// go for more
        }
	break;					// exit loop
      }
    }
    if (j > 255)				// too many dots
    { comperror(-(ERRMLAST+ERRZ13));		// complain
      continue;					// go for more
    }
    if (j || last_dots)				// any dots (or on last line)
    { *comp_ptr++ = CHKDOTS;			// the op code
      *comp_ptr++ = (u_char) j;			// the count
      last_dots = j;				// remember for ron
    }

    if ((*source_ptr != ';') &&			// ignore comment lines
	(*source_ptr != '\0'))			// ignore empty lines
      parse();					// parse the line
    else
      *comp_ptr++ = ENDLIN;			// end of null line
    *((u_short *)p) = (short) (comp_ptr - p) - 1; // offset to ENDLIN

  }						// end main compile loop
  *comp_ptr++ = CMQUIT;				// mark end of routine
  *comp_ptr++ = ENDLIN;				// mark end of routine
  *comp_ptr++ = ENDLIN;				// mark end of routine
  partab.varlst = NULL;				// for localvar()
  for (num_vars = 0; var_tbl[num_vars].var_qu != 0; num_vars++); // count them
  p = line->buf;				// where we put it now

  cptr->len = Vhorolog(cptr->buf);		// get current date/time
  s = COMP_VER;
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = COMP_VER;			// compiler version
  s = partab.jobtab->user;
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = partab.jobtab->user;	// user who compiled it
  i = cstringtoi(cptr);
  bcopy(&i, p, 4);
  p += 4;
  //*((int *)p)++ = cstringtoi(cptr);		// get the date
  i = atoi((char *)&cptr->buf[6]);
  bcopy(&i, p, 4);
  p += 4;
  //*((int *)p)++ = atoi((char *)&cptr->buf[6]);		// and the time
  i = sizeof(tags) * num_tags;			// space for tags
  j = sizeof(var_u) * num_vars;			// space for vars
  s = (p - line->buf) + (6 * sizeof(u_short));	// offset for tags
  bcopy(tag_tbl, &line->buf[s], i);		// copy tag table
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = s;			// where it went
  ss = (short)num_tags;
  bcopy(&ss, p, 2);
  p += 2;
  //*((u_short *)p)++ = num_tags;			// copy the count
  s = s + i;					// wher vars go
  bcopy(var_tbl, &line->buf[s], j);		// copy var table
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = s;			// where it went
  ss = (short) num_vars;
  bcopy(&ss, p, 2);
  p += 2;
  //*((u_short *)p)++ = num_vars;			// copy the count
  s = s + j;					// where the code goes
  bcopy(code, &line->buf[s], comp_ptr - code);	// copy the code
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = s;			// where it went
  s = (short) (comp_ptr - code);
  bcopy(&s, p, 2);
  p += 2;
  //*((u_short *)p)++ = (comp_ptr - code);	// and the size
  i = p - line->buf + (comp_ptr - code) + i + j; // total size
  if (i > MAX_STR_LEN) 
  { comperror(-(ERRZ54+ERRMLAST));		// complain
    if (!partab.checkonly) SemOp(SEM_ROU, systab->maxjob); // unlock
    return -ERRM75;				// ignore the rest
  }
  line->len = i;				// save the length
  cptr->buf[0] = '0';				// where it goes
  cptr->buf[1] = '\0';				// null terminated
  cptr->len = 1;				// the size
  if (partab.checkonly)				// just a check
    return 0;					// exit - NEED an error count
  s = UTIL_Key_Build(cptr, &rou->key[rou_slen]); // build the key
  rou->slen = rou_slen + s;			// store the new length
  s = DB_Set(rou, line);			// set it
  if (same)
    Routine_Delete(rounam.var_qu, rou->uci);	// delete the routine
  i = SemOp(SEM_ROU, systab->maxjob);		// release sem
  return s;					// NEED MORE HERE
}
