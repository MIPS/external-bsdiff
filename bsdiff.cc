/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
__FBSDID("$FreeBSD: src/usr.bin/bsdiff/bsdiff/bsdiff.c,v 1.1 2005/08/06 01:59:05 cperciva Exp $");
#endif

#include "bsdiff/bsdiff.h"

#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

#include "patch_writer.h"

namespace bsdiff {

static off_t matchlen(const u_char* old, off_t oldsize, const u_char* new_buf,
                      off_t newsize) {
	off_t i;

	for(i=0;(i<oldsize)&&(i<newsize);i++)
		if(old[i]!=new_buf[i]) break;

	return i;
}

// This is a binary search of the string |new_buf| of size |newsize| (or a
// prefix of it) in the |old| string with size |oldsize| using the suffix array
// |I|. |st| and |en| is the start and end of the search range (inclusive).
// Returns the length of the longest prefix found and stores the position of the
// string found in |*pos|.
static off_t search(saidx_t* I, const u_char* old, off_t oldsize,
                    const u_char* new_buf, off_t newsize, off_t st, off_t en,
                    off_t* pos) {
	off_t x,y;

	if(en-st<2) {
		x=matchlen(old+I[st],oldsize-I[st],new_buf,newsize);
		y=matchlen(old+I[en],oldsize-I[en],new_buf,newsize);

		if(x>y) {
			*pos=I[st];
			return x;
		} else {
			*pos=I[en];
			return y;
		}
	};

	x=st+(en-st)/2;
	if(memcmp(old+I[x],new_buf,std::min(oldsize-I[x],newsize))<=0) {
		return search(I,old,oldsize,new_buf,newsize,x,en,pos);
	} else {
		return search(I,old,oldsize,new_buf,newsize,st,x,pos);
	};
}

int bsdiff(const char* old_filename, const char* new_filename,
           const char* patch_filename) {
	int fd;
	u_char *old_buf,*new_buf;
	off_t oldsize,newsize;

	/* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd=open(old_filename,O_RDONLY,0))<0) ||
		((oldsize=lseek(fd,0,SEEK_END))==-1) ||
		((old_buf=static_cast<u_char*>(malloc(oldsize+1)))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,old_buf,oldsize)!=oldsize) ||
		(close(fd)==-1)) err(1,"%s",old_filename);

	/* Allocate newsize+1 bytes instead of newsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd=open(new_filename,O_RDONLY,0))<0) ||
		((newsize=lseek(fd,0,SEEK_END))==-1) ||
		((new_buf = static_cast<u_char*>(malloc(newsize+1)))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,new_buf,newsize)!=newsize) ||
		(close(fd)==-1)) err(1,"%s",new_filename);

	int ret = bsdiff(old_buf, oldsize, new_buf, newsize, patch_filename, nullptr);

	free(old_buf);
	free(new_buf);

	return ret;
}

// Generate bsdiff patch from |old_buf| to |new_buf|, save the patch file to
// |patch_filename|. Returns 0 on success.
// |I_cache| can be used to cache the suffix array if the same |old_buf| is used
// repeatedly, pass nullptr if not needed.
int bsdiff(const u_char* old_buf, off_t oldsize, const u_char* new_buf,
           off_t newsize, const char* patch_filename, saidx_t** I_cache) {
	saidx_t *I;
	off_t scan,pos=0,len;
	off_t lastscan,lastpos,lastoffset;
	off_t oldscore,scsc;
	off_t s,Sf,lenf,Sb,lenb;
	off_t overlap,Ss,lens;
	off_t i;
	BsdiffPatchWriter patch(old_buf, oldsize, new_buf, newsize);

	if (I_cache && *I_cache) {
		I = *I_cache;
	} else {
		if ((I=static_cast<saidx_t*>(malloc((oldsize+1)*sizeof(saidx_t))))==NULL)
			err(1,NULL);

		if (divsufsort(old_buf, I, oldsize)) err(1, "divsufsort");
		if (I_cache)
			*I_cache = I;
	}

	/* Create the patch file */
	if (!patch.Open(patch_filename))
		return 1;

	/* Compute the differences, writing ctrl as we go */
	scan=0;len=0;
	lastscan=0;lastpos=0;lastoffset=0;
	while(scan<newsize) {
		oldscore=0;

		/* If we come across a large block of data that only differs
		 * by less than 8 bytes, this loop will take a long time to
		 * go past that block of data. We need to track the number of
		 * times we're stuck in the block and break out of it. */
		int num_less_than_eight = 0;
		off_t prev_len, prev_oldscore, prev_pos;
		for(scsc=scan+=len;scan<newsize;scan++) {
			prev_len=len;
			prev_oldscore=oldscore;
			prev_pos=pos;

			len=search(I,old_buf,oldsize,new_buf+scan,newsize-scan,
					0,oldsize-1,&pos);

			for(;scsc<scan+len;scsc++)
			if((scsc+lastoffset<oldsize) &&
				(old_buf[scsc+lastoffset] == new_buf[scsc]))
				oldscore++;

			if(((len==oldscore) && (len!=0)) ||
				(len>oldscore+8)) break;

			if((scan+lastoffset<oldsize) &&
				(old_buf[scan+lastoffset] == new_buf[scan]))
				oldscore--;

			const off_t fuzz = 8;
			if (prev_len-fuzz<=len && len<=prev_len &&
			    prev_oldscore-fuzz<=oldscore &&
			    oldscore<=prev_oldscore &&
			    prev_pos<=pos && pos <=prev_pos+fuzz &&
			    oldscore<=len && len<=oldscore+fuzz)
				++num_less_than_eight;
			else
				num_less_than_eight=0;
			if (num_less_than_eight > 100) break;
		};

		if((len!=oldscore) || (scan==newsize)) {
			s=0;Sf=0;lenf=0;
			for(i=0;(lastscan+i<scan)&&(lastpos+i<oldsize);) {
				if(old_buf[lastpos+i]==new_buf[lastscan+i]) s++;
				i++;
				if(s*2-i>Sf*2-lenf) { Sf=s; lenf=i; };
			};

			lenb=0;
			if(scan<newsize) {
				s=0;Sb=0;
				for(i=1;(scan>=lastscan+i)&&(pos>=i);i++) {
					if(old_buf[pos-i]==new_buf[scan-i]) s++;
					if(s*2-i>Sb*2-lenb) { Sb=s; lenb=i; };
				};
			};

			if(lastscan+lenf>scan-lenb) {
				overlap=(lastscan+lenf)-(scan-lenb);
				s=0;Ss=0;lens=0;
				for(i=0;i<overlap;i++) {
					if(new_buf[lastscan+lenf-overlap+i]==
					   old_buf[lastpos+lenf-overlap+i]) s++;
					if(new_buf[scan-lenb+i]==
					   old_buf[pos-lenb+i]) s--;
					if(s>Ss) { Ss=s; lens=i+1; };
				};

				lenf+=lens-overlap;
				lenb-=lens;
			};

			if (!patch.AddControlEntry(ControlEntry(lenf,
			                                        (scan - lenb) - (lastscan + lenf),
			                                        (pos - lenb) - (lastpos + lenf))))
				errx(1, "Writing a control entry");

			lastscan=scan-lenb;
			lastpos=pos-lenb;
			lastoffset=pos-scan;
		};
	};
	if (!patch.Close())
		errx(1, "Closing the patch file");

	if (I_cache == nullptr)
		free(I);

	return 0;
}

}  // namespace bsdiff
