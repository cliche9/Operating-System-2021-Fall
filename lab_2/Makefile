srcs = tpipe.c
objs = tpipe.o
opts = -g -c
all: tpipe
pctl: $(objs)
	gcc $(objs) -l pthread -o tpipe
tpipe.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm tpipe *.o