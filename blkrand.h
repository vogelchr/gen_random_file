/*
 *  gen_random_file - Generate a file with random content.
 *
 *  Written in 2014 by Christian Vogel <vogelchr@vogel.cx>.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted without restrictions.
 */

#ifndef BLKRAND_H
#define BLKRAND_H

#include <stdlib.h>	/* for size_t */

/* fill a block of memory with random numbers.
   blkrand_init() initializes the seed of the RNG
   blkrand_fill(buf, size) fills a buffer with data.

   Note that buf must be aligned to 16 bytes and size must be evenly
   divisable by 16!

   blkrand_fill() will do nothing, and return -1 if both conditions
   aren't met.

   It will return 0 if verything went fine.
*/

extern int
blkrand_init();

extern int
blkrand_fill(char *buf, size_t nbytes, unsigned char *hash);

#endif
