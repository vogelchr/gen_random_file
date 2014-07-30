/*
 *  gen_random_file - Generate a file with random content.
 *
 *  Written in 2014 by Christian Vogel <vogelchr@vogel.cx>.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted without restrictions.
 */


#include "blkrand.h"
#include "fastrand.h"
#include "sha1.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

struct fastrand_init_vars {
	uint16_t a1; uint16_t c1;
	uint16_t b1; uint16_t d1;
	uint16_t a2; uint16_t c2;
	uint16_t b2; uint16_t d2;
	uint16_t a3; uint16_t c3;
	uint16_t b3; uint16_t d3;
	uint16_t a4; uint16_t c4;
	uint16_t b4; uint16_t d4;
};

static fastrand f;

int
blkrand_init()
{
	int fd;
	struct fastrand_init_vars fiv;

	if (-1 == (fd=open("/dev/urandom", O_RDONLY))) {
		perror("/dev/urandom");
		return -1;
	}

	if (read(fd, &fiv, sizeof(fiv)) != sizeof(fiv)) {
		perror("/dev/urandom");
		close(fd);
		return -1;
	}

	if (close(fd))
		return -1;

	InitFastRand(fiv.a1, fiv.c1, fiv.b1, fiv.d1,
		     fiv.a2, fiv.c2, fiv.b2, fiv.d2,
		     fiv.a3, fiv.c3, fiv.b3, fiv.d3,
		     fiv.a4, fiv.c4, fiv.b4, fiv.d4, &f);

	return 0;
}

#define BYTES_PER_LOOP	sizeof(f.res)
/* assume that BYTES_PER_LOOP is a simple power of two! */
#define ALIGNMENT_MASK  (BYTES_PER_LOOP-1)
#define ALIGN_SHA1	0xffff		/* 64k per sha1-loop */

int
blkrand_fill(char *outbuf, size_t nbytes, unsigned char *hashout)
{
	/* FastRand always generates 4 32bit numbers at a time, so
	   first make sure our base is 16 byte aligned */
	char *end;         /* end of copying */
	char *last_sha1;
	blk_SHA_CTX sha_ctx;

	end = outbuf + nbytes;

	/* force the buffer to be aligned, it's annoying otherwise */
	if ((unsigned long)outbuf & ALIGNMENT_MASK)
		return -1;
	if ((unsigned long)end & ALIGNMENT_MASK)
		return -1;

	if (hashout)
		blk_SHA1_Init(&sha_ctx);

	last_sha1 = outbuf;
	while (outbuf != end) {
		FastRand_SSE(&f);
		memcpy(outbuf, &f.res, BYTES_PER_LOOP);

		outbuf += BYTES_PER_LOOP;

		if (hashout) {
			if (((unsigned long)outbuf & ALIGN_SHA1) == 0) {
				blk_SHA1_Update(&sha_ctx, last_sha1, outbuf-last_sha1);
				last_sha1 = outbuf;
			}
		}

	}

	if (hashout && (last_sha1 != outbuf))
		blk_SHA1_Update(&sha_ctx, last_sha1, outbuf-last_sha1);

	if (hashout)
		blk_SHA1_Final(hashout, &sha_ctx);

	return 0;
}
