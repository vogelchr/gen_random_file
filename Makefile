CFLAGS=-Wall -Wextra -Os -ggdb
CPPFLAGS=
OBJS=gen_random_file.o blkrand.o

all : gen_random_file

gen_random_file : $(OBJS)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif

%.d : %.c
	$(CC) $(CPPFLAGS) -o $@ -MM $<

.PHONY : clean install
clean :
	rm -f *~ *.o *.d gen_random_file

install :
	install -m755 -o0 -g0 gen_random_file /usr/local/bin
