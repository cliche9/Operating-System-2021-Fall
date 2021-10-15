srcs = lab_2.c
objs = lab_2.o
opts = -g -c
all: lab_2
lab_2: $(objs)
	gcc $(objs) -l pthread -o lab_2
lab_2.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm lab_2 *.o