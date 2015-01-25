// File: mumps/util/util_key.c
//
// module database - Key Utilities

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
#include <string.h>				// for memcpy/memcmp
#include <strings.h>
#include <sys/types.h>				// for u_char def
#include "mumps.h"                              // standard includes
#include "proto.h"                              // standard prototypes
#include "error.h"                              // errors
#include "compile.h"				// for rbd definition

//*** Function: UTIL_Key_Build - Build a key from an ascii source *********
//**  Key is in order: NEGATIVE NUMBER -> ZERO -> POSITIVE NUMBER -> STRING
//*************************************************************************
short UTIL_Key_Build( cstring *src,             // locn of source string
                      u_char *dest)             // where to put it
{ int minus = 0;                                // minus flag
  int dp = -1;                                  // decimal point flag
  int to = 0;					// for dst[]
  int idx = 0;					// for src->buf[]
  int i;                                        // for loops

  if ((src->len < 0) || (src->len > 127))       // neg or > 127 is illegal
  { return -(ERRZ1+ERRMLAST);			// complain
  }

  if (src->len == 0)                            // test for null string
  { dest[to++] = '\0';				// move in a null
    dest[to++] = '\0';				// and another
    dest[to] = '\0';				// null terminate it
    return (short) to;                 		// return the count
  }                                             // end null string code

  if ((src->len == 1) && (src->buf[0] == '0'))  // test for a zero
  { dest[to++] = 64;                            // move in $C(64)
    dest[to++] = '\0';                 		// and the null
    dest[to] = '\0';				// null terminate it
    return (short) to;                 		// return the count
  }                                             // end zero code

  if (src->buf[idx] == '-')                     // if it's negative
  { idx++;                                      // increment pointer
    minus = 1;                                  // flag minus
  }                                             // end skip "-"
  if (src->buf[idx] == '.')
  { dp = 0;             			// check for starting dp
  }
  else if ((src->buf[idx] < '1') || (src->buf[idx] > '9'))
  { goto string;                                // check for a string key
  }
  for ( i = idx + 1; i < src->len; i++ )	// remainder of string
  { if (src->buf[i] == '.')                     // check for decmal point
    { if (dp != -1)
      { goto string;				// second dp is out
      }
      dp = i - minus;				// save dp position
    }                                           // end dp code
    else if ((src->buf[i] < '0') || (src->buf[i] > '9'))
    { goto string;                              // exit if not numeric
    }
  }                                             // end numeric check
  if ((dp != -1) && (src->buf[src->len-1] == '0'))
  { goto string;				// check trailing 0 after dp
  }
  if (dp == (src->len-1))
  { goto string;          			// or dp is last char in str
  }
  if (dp == -1)
  { dp = (src->len - minus);        		// get dp posn (assumed)
  }
  if (dp > 63)
  { goto string;                     		// max 63 digits b4 dp
  }

  if (!minus)                                   // do a positive number
  { dest[to++] = (u_char)(dp + 64);             // copy in the count + flag
    for (i = 0; i < dp; i++)
    { dest[to++] = src->buf[i];  		// copy up to dp
    }
    for (i = dp + 1; i < src->len; i++)
    { dest[to++] = src->buf[i]; 		// copy remainder
    }
    dest[to++] = '\0';                          // trailing null for +ve
    dest[to] = '\0';				// null terminate it
    return (short) to;                 		// return the count
  }                                             // end of positive code

  dest[to++] = (u_char)(63 - dp);               // copy in 1s comp of count
  for ( i = idx; i < src->len; i++)             // go thru the string
  { if (src->buf[i] != '.')                     // ignore the dp
    { dest[to++] = (u_char)(57 - src->buf[i] + 48); // nines complement
    }
  }
  dest[to++] = 255;                             // trailing -1
  dest[to] = '\0';				// null terminate it
  return (short) to;                 		// return the count

// The following is the string code

string:						// do a string key

  dest[to++] = (u_char) 128;                	// copy in the flag
  for (i = 0; i < src->len; i++)   		// for each char in string
  { if ((dest[to++] = src->buf[i]) == 0)	// copy it
    { return -(ERRZ5+ERRMLAST);			// complain if null
    }
  }
  dest[to++] = '\0';				// trailing null
  dest[to] = '\0';				// null terminate it
  return (short) to;                 		// return the count
}                                               // end of Key_Build()


//****  Function: UTIL_Key_Extract - Extract a key from a key record ***
//**  Updates cnt (count of characters at key).                      ***
//**********************************************************************
short UTIL_Key_Extract( u_char *key,            // where the key is
                        u_char *str,            // locn of dest string
                        int *cnt)               // addr of count of chars used

	// NOTE: if cnt is passed in non-zero, double the quotes(")

{ int s;                                        // size
  int i = 0;					// index
  int j = 0;					// string count
  int idx = 0;					// and another
  int flg;					// flag for quotes in string
  flg = *cnt;					// get the flag
  s = *key++;                                   // get first char
  if (((s == 0) || (s == 255)) &&
      ( *key == 0))				// check for null str
  { *cnt = 2;					// used 2 bytes
    str[0] = '\0';				// nul term
    return 0;
  }
  if (s & 128)                                  // if it's a string
  { for (i = 0; key[i] != 0; i++)		// loop thru
    { str[j++] = key[i];			// copy till done
      if ((key[i] == '"') && (flg)) str[j++] = '"'; // double quote if reqd
      if (i > 127) return -(ERRZ1+ERRMLAST);	// check size
    }
    str[j] = 0;					// null terminate
    *cnt = i+2;					// store bytes used
    return (short) j;				// return string count
  }                                             // end of string processing

  if (s & 64)                                   // if it's a positive number
  { s &= 63;                                    // extract dp position
    if ((*key == '\0') && (s == 0))		// check for numeric 0
    { str[idx++] = '0';				// add zero
      str[idx] = '\0';				// null terminate
      *cnt = 2;					// used 2
      return (short) idx;			// return length (1)
    }
    for (i = 0; i < s; i++) str[idx++] = *key++; // copy to dp
    str[idx] = 0;				// null term (in case)
    *cnt = s+2;	                                // assume no dp, save count
    if (*key == '\0') return (short) idx;       // if char 0, all done
    str[idx++] = '.';                           // add the dp
    while ((str[idx++] = *key++)) s++;          // move to NULL, counting
    --idx;					// back to point at NULL
    if (s > 127) return -(ERRZ1+ERRMLAST);      // check size
    *cnt = s + 2;                               // update count
    return (short) idx;                         // return string count
  }                                             // end of positive number

  s = 63 - s;                                   // get negative count
  str[idx++] = '-';                             // save minus sign
  for (i = 0; i < s; i++)                       // copy to dp
    str[idx++] = ('9' + '0' -*key++);           // doing a 9's complement
  str[idx] = 0;					// null term (in case)
  *cnt = s + 2;                                 // update the count
  if (*key == 255) return (short) idx;          // if char 255, all done
  str[idx++] = '.';                             // add the dp
  while (TRUE)                                  // loop for end
  { if (*key == 255) break;                     // check for end of string
    s++;                                        // count character
    str[idx++] = ('9' + '0' -*key++);           // copy a 9's complement
  }                                             // end while
  if (s > 127) return -(ERRZ1+ERRMLAST);        // check size
  str[idx] = 0;					// null term
  *cnt = s + 2;                                 // update count
  return (short) idx;                           // return new key pointer
}

//****  Function: UTIL_String_Key - Extract all keys to string ***
//****************************************************************
short UTIL_String_Key( u_char *key,             // where the key is -> count
                       u_char *str,             // locn of dest string
                       int max_subs) 		// max number of subscripts
{ int count = 1;				// bytes used and quote flag
  int len;					// bytes in key
  int idx = 0;					// key index
  int clen = 0;					// len of returned string
  int string = 0;				// string indicator
  short ret;					// return value
  len = (int) key[idx++];			// get key length
  str[clen++] = '(';				// open bracket
  while (len > 1)				// while there are chars in key
  { string = 0;					// clear string ind
    if (key[idx] & 128)            		// if it's a string
    { string = 1;				// flag it
      str[clen++] = '"';			// add leading quote
    }
    ret = UTIL_Key_Extract( &key[idx], &str[clen], &count); //get one key
    if (ret < 0) return ret;			// die on error
    if (ret == 0)				// nul key
    { string = 1;				// flag it as a string
      str[clen++] = '"';			// add leading quote
    }
    clen = clen + (int) ret;			// add to string length
    if (string == 1) str[clen++] = '"';		// add trailing quote
    len = len-count;				// subtract used bytes
    idx = idx + count;				// adjust key index
    str[clen++] = ',';				// add a comma
    max_subs--;					// count subscript
    if (max_subs < 1) break;			// give up if all done
  }
  clen--;					// last comma
  str[clen++] = ')';				// replace with )
  str[clen] = 0;				// null terminate
  return (short) clen;				// and return the length
}

//****************************************************************
int UTIL_Key_Last( mvar *var)			// point at last subs in mvar
{ int last = -1;				// return value
  int i = 0;					// idx into var->key[]

  while (i < var->slen)				// while any there
  { last = i;					// save beginning
    if (var->key[i++] < 64)			// negative number
      while ((var->key[i++] != 255) && (i < var->slen)); // scan to end
    else					// positive or string
      while ((var->key[i++] != 0) && (i < var->slen));	// scan to end
  }
  return last;					// return the index
}

//****************************************************************
short UTIL_String_Mvar( mvar *var,            	// address of mvar
                        u_char *str,            // locn of dest string
                        int max_subs)		// max number of subscripts
{ int i;					// for loops
  int p = 0;					// string pointer
  int vol;					// for volset
  uci_tab up;					// ptr to uci tab
  chr_q *vt;					// var table pointer
  rbd *r;					// a handy pointer
  u_char *ptr;					// string ptr

  if (var->uci != UCI_IS_LOCALVAR)		// if it's a global var
  { str[p++] = '^';				// lead off with the caret
    if (var->uci != 0)				// if an environment specified
    { str[p++] = '[';				// open bracket
      str[p++] = '"';				// a leading quote
      vol = var->volset;			// get vol
      if (vol == 0) vol = partab.jobtab->vol;	// if none, get default
      up = systab->vol[vol-1]->vollab->uci[var->uci-1]; // uci tab pointer
      for (i=0; i<8; i++)			// for each possible character
      { if (up.name.var_cu[i] == '\0') break;	// done if we hit a null
        str[p++] = up.name.var_cu[i];		// copy the character
      }
      str[p++] = '"';				// a trailing quote
      if (var->volset != 0)			// volset specified?
      { str[p++] = ',';				// copy in a comma
        str[p++] = '"';				// a leading quote
	ptr = systab->vol[var->volset-1]->vollab->volnam.var_cu;
        for (i=0; i<8; i++)			// for each possible character
        { if (ptr[i] == '\0') break;		// done if we hit a null
          str[p++] = ptr[i];			// copy the character
        }
        str[p++] = '"';				// a trailing quote
      }
      str[p++] = ']';				// closing bracket
    }						// end environment stuf
  }						// end global specific stuf
  if ((var->uci == UCI_IS_LOCALVAR) &&
      (var->volset))				// special index type
  { r = (rbd *) (partab.jobtab->dostk[partab.jobtab->cur_do].routine);
    vt = (chr_q *) (((u_char *) r) + r->var_tbl); // point at var table
    var->name.var_qu = vt[var->volset - 1];	// get the var name
    var->volset = 0;				// clear the volset
  }

  for (i = 0; i < 8; i++)			// now the name
  { if (var->name.var_cu[i] == '\0') break;	// quit when done
    str[p++] = var->name.var_cu[i];		// copy a byte
  }

  if ((var->slen != 0) && (max_subs > 0))	// if there are subscripts
  { i = UTIL_String_Key( &var->slen, &str[p], max_subs); //do the subscripts
    if (i < 0) return i;			// quit on error
    p = p + i;					// add to length
  }
  str[p] = '\0';				// null terminate
  return (short) p;				// return the length
}

//****************************************************************
// returns number of subscripts or negative error message
//
short UTIL_MvarFromCStr( cstring *src,		// the string
                         mvar *var)		// destination mvar
{ int i;					// a handy int
  int q;					// for quotes
  short s;					// for functions
  int subs = 0;					// number of subscripts
  u_char *ptr;					// a handy pointer
  cstring *kb;					// for key builds
  var_u nam;					// for name comparisons
  var_u vol;					// ditto
  u_char tmp[260];				// temp area for subscripts

  kb = (cstring *) tmp;				// make it a cstring
  var->volset = 0;				// clear volset
  var->uci = UCI_IS_LOCALVAR;			// assume local variable
  var->slen = 0;				// and no subscripts
  var->name.var_qu = 0;				// clear the name
  ptr = src->buf;				// point at the source
  if (*ptr == '^')				// global?
  { ptr++;					// skip the ^
    var->uci = 0;				// not a local var

    if (*ptr == '[')				// environment specified?
    { ptr++;					// skip the [
      nam.var_qu = 0;				// clear quadword
      if (*ptr++ != '"')			// must be a quote
        return -(ERRMLAST+ERRZ12);		// complain
      i = 0;					// clear an index
      while (*ptr != '"')			// scan to end of literal
      { if (i == 8)				// check for too many
          return -(ERRMLAST+ERRZ12);		// complain
        nam.var_cu[i++] = *ptr++;		// copy a byte
      }
      ptr++;					// go past closing "

      if (*ptr == ',')				// vol name too ?
      { ptr++;					// skip the ,
        vol.var_qu = 0;				// clear quadword
        if (*ptr++ != '"')			// must be a quote
          return -(ERRMLAST+ERRZ12);		// complain
        i = 0;					// clear an index
        while (*ptr != '"')			// scan to end of literal
        { if (i == 8)				// check for too many
            return -(ERRMLAST+ERRZ12);		// complain
          vol.var_cu[i++] = *ptr++;		// copy a byte
        }
        ptr++;					// go past closing "
        for (i = 0; i < MAX_VOL; i++)		// scan vol list
          if (systab->vol[i] != NULL)		// vol here ?
            if (systab->vol[i]->vollab->volnam.var_qu == vol.var_qu)
	      break;				// quit if found
        if (i == MAX_VOL) return -ERRM26;	// no such, complain
        var->volset = i + 1;			// store the vol#
      }
      if (var->volset == 0) var->volset = partab.jobtab->vol; // default
      for (i = 0; i < UCIS; i++)		// scan uci list (vol 0)
        if (systab->vol[var->volset-1]->vollab->uci[i].name.var_qu
              == nam.var_qu)
	  break;				// quit if found
      if (i == UCIS) return -ERRM26;		// no such, complain
      var->uci = i + 1;				// store the uci#
      if (*ptr++ != ']')			// don't allow volume for now
        return -ERRM26;				// give error instead
    }
  }						// end special global stuff
  for (i = 0; i < 8; i++)			// now the name
  { if ((*ptr == '(') || (*ptr == '\0'))	// subs or end of str
      break;					// quit
    var->name.var_cu[i] = *ptr++;		// copy a byte
  }
  if (*ptr == '\0') return 0;			// end of string - all done
  if (*ptr++ != '(')				// must be a (
    return -(ERRMLAST+ERRZ12);			// complain
  while (TRUE)					// till we run out of subs
  { if (*ptr == '\0') return -(ERRMLAST+ERRZ12); // junk
    q = (*ptr == '"');				// check for quotes
    if (q) ptr++;				// skip the quote
    i = 0;					// init index
    while (TRUE)				// move 1 subs
    { if (*ptr == '\0') return -(ERRMLAST+ERRZ12); // junk
      if (*ptr == '"' && q)			// quote
      { ptr++;					// skip it
        if (*ptr != '"')			// next not a quote
	{ if (*ptr == ',') break;		// done
	  if (*ptr == ')') break;		// also done
          return (-(ERRMLAST+ERRZ12));		// junk
	}
      }						// end quote processing
      if (i == 255) return -(ERRMLAST+ERRZ12);	// junk
      if ((!q) && ((*ptr == ',') || (*ptr == ')'))) // end numeric subs
        break;					// done with this one
      kb->buf[i++] = *ptr++;			// copy one character
    }						// end copy 1 subs
    kb->buf[i] = '\0';				// null terminate
    kb->len = i;				// save the length
    s = UTIL_Key_Build(kb, &var->key[var->slen]); // do one key
    if (s < 0) return s;			// got an error
    if ((s + var->slen) > 255) return -(ERRMLAST+ERRZ12); // junk
    if ((var->key[var->slen] == 128) && (!q))	// got a string + no quotes
      return -(ERRMLAST+ERRZ12);		// junk
    subs++;					// count a subscript
    var->slen = s + var->slen;			// save new length
    if (*ptr == ',')				// comma?
    { ptr++;					// skip it
      continue;					// and keep going
    }
    if (*ptr == ')')				// trailing bracket
    { ptr++;					// skip it
      break;					// and quit
    }
    return -(ERRMLAST+ERRZ12);			// junk
  }
  if (*ptr != '\0') return -(ERRMLAST+ERRZ12);	// junk
  return subs;					// all OK
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Function:		UTIL_Key_KeyCmp
// Description: 	Compares the keys
// Return Values:	K2_LESSER, K2_GREATER, KEQUAL.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int UTIL_Key_KeyCmp(u_char *key1, u_char *key2, int kleng1, int kleng2)
{ int cmpvar;					// comparison variable

  cmpvar = memcmp(key1, key2,
           (kleng1 < kleng2) ? kleng1: kleng2); // compare keys
  if (!cmpvar)					// if start of keys is same
  { if (kleng1 == kleng2)			// and the lengths are equal
    { return (KEQUAL);				// ...keys are the same
    }
    if (kleng1 > kleng2)			// if length of key 1 if bigger
    { return (K2_LESSER);			// ...key1 sorts after key2
    }
    return (K2_GREATER);			// ...key2 sorts after key1
  }
  if (cmpvar > 0)				// if value of key1 is greater
  { return (K2_LESSER);				// ...key1 sorts after key2
  }
  return (K2_GREATER);				// ...key2 sorts after key1
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Function:		Chars_In_Subs
// Description:		Count the chars in a number of subscripts of a key.
//			Will also return number of subs and copy those subs
//			if either (int *) or last (char *) is not NULL.
// Return Values:	Number of chars in subscripts counted.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int UTIL_Key_Chars_In_Subs( char *Key, int keylen, int maxsubs, int *subs,
                                                        char *KeyBuffer )
{ int cnt, i;						// subs & char counts
  cnt = 0;						// initialise
  i = 0;						// these two
  while ( ( i < keylen ) && ( cnt < maxsubs ) )		// while still in key
  { if ( (Key[i]&128) || (Key[i]&64) )			// if +ve no. or string
    { for (i++; Key[i]; i++);				// loop til find NULL
        i++;						// skip NULL char
    }
    else						// else if -ve
    { for (i++; (Key[i] != -1); i++);			// loop til find $C(255)
        i++;						// skip past 255
    }
    cnt++;						// increment subs count
  }
  if (subs != NULL)					// if we should remember
    *subs = cnt;					// subs, then copy
  if (KeyBuffer != NULL)				// if we want the chars
    bcopy( Key, KeyBuffer, i);				// then copy them
  return (i);						// return no. of chars
}
