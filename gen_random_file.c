#include "blkrand.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 *  gen_random_file - Generate a file with random content.
 *
 *  Written in 2014 by Christian Vogel <vogelchr@vogel.cx>.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted without restrictions.
 */

static int force_overwrite=0;
static int verbose=0;

static void
usage(char *argv0)
{
	fprintf(stderr,"Usage: %s [options] size[kMG] filename\n", argv0);
	fprintf(stderr,"  Create a file with random content of specific size.");
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -h     : this help\n");
	fprintf(stderr,"  -v     : be verbose\n");
	fprintf(stderr,"  -f     : force overwrite\n");
}

int
main(int argc, char **argv)
{
	int i;
	int fd;
	size_t sz;
	void *fmap;
	int bitshift = 0;

	while (-1 != (i=getopt(argc, argv, "hvf"))) {
		switch (i) {
		case 'h':
			usage(argv[0]);
			exit(1);
		case 'v':
			verbose++;
			break;
		case 'f':
			force_overwrite++;
			break;
		}
	}

	if (argc != optind+2) {
		usage(argv[0]);
		exit(1);
	}

	/* parse size */
	i = strlen(argv[optind]); /* prefix kMG */
	switch (argv[optind][i-1]) {
	case 'k':
		bitshift=10;
		break;
	case 'M':
		bitshift=20;
		break;
	case 'G':
		bitshift=30;
		break;
	default:
		fprintf(stderr,"Cannot parse \"%s\" as size.\n",argv[optind]);
		fprintf(stderr,"Please specify a size suffix of k,M or G only.\n");
		usage(argv[0]);
		exit(1);
	}

	argv[optind][i]='\0';
	sz = atoi(argv[optind]);
	sz <<= bitshift;

	if (verbose)
		fprintf(stderr,"File is %s, size is %zu bytes.\n",
			argv[optind+1],sz);

	blkrand_init();

	/* open and mmap file */

	i = O_RDWR|O_EXCL|O_CREAT;
	if (force_overwrite)	/* -f flag allows overwriting of existing */
		i &= ~ O_EXCL;	/*    files */

	if (-1 == (fd=open(argv[optind+1], i,0644))) {
		perror(argv[optind+1]);
		close(fd);
		exit(1);
	}

	if (-1 == ftruncate(fd, sz)) {
		perror(argv[optind+1]);
		close(fd);
		exit(1);		
	}

	fmap = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (fmap == MAP_FAILED) {
		perror(argv[optind+1]);
		close(fd);
		exit(1);
	}

	if (-1 == close(fd)) {
		perror(argv[optind+1]);
		exit(1);
	}

	if (-1 == blkrand_fill(fmap, sz)) {
		fprintf(stderr,"blkrand() error!\n");
		exit(1);
	}

	if (-1 == munmap(fmap, sz)) {
		perror(argv[optind+1]);
		exit(1);		
	}

	return 0;
}
