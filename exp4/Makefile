hdrs = ipc.h
opts = -g -c
c_src = smoker.c ipc.c 
c_obj = smoker.o ipc.o 
p_src = supplier.c ipc.c 
p_obj = supplier.o ipc.o 
all: 	supplier smoker
smoker: $(c_obj)
	gcc $(c_obj) -o smoker
smoker.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src)
supplier: $(p_obj)
	gcc $(p_obj) -o supplier
supplier.o: $(p_src) $(hdrs) 
	gcc $(opts) $(p_src)
clean:
	rm smoker supplier *.o
