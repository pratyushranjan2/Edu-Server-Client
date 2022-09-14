EXECS= srv clt

all: $(EXECS)

srv: server.c utils.c utils.h sigtypes.h configs.h
	gcc -o srv -D REENTRANT server.c utils.c -lrt -lpthread

clt: client.c utils.c utils.h configs.h
	gcc -o clt client.c utils.c -lrt
	
clean:
	rm -f $(EXECS)
