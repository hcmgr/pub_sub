CC=clang
FLAGS=-g -fPIC -pedantic -Wall -std=gnu99
A4_LIB=-lcsse2310a4
A3_LIB=-lcsse2310a3
LIB_STRING_MAP_LIB=-lstringmap
PTHREAD=-pthread

# all: shared lock stats client server libstringmap.so
all: shared lock stats libstringmap.so client server

client: client.c shared.o
	$(CC) $(FLAGS) -L. $(A4_LIB) ${A3_LIB} $(PTHREAD) \
	# $(CC) $(FLAGS) $(PTHREAD) \
	    -o psclient $^ 

server: server.c clientList.c shared.o lock.o stats.o
	$(CC) $(FLAGS) -L. $(LIB_STRING_MAP_LIB) $(A4_LIB) $(A3_LIB) $(PTHREAD) \
	# $(CC) $(FLAGS) $(PTHREAD) \
	    -o psserver $^ 

shared: shared.c 
	$(CC) $(FLAGS) -c -o shared.o $^

lock: lock.c
	$(CC) $(FLAGS) -c -o lock.o $^

stats: stats.c 
	$(CC) $(FLAGS) -c -o stats.o $^

stringmap.o: stringmap.c
	$(CC) $(FLAGS) -fPIC  \
	-c $<

libstringmap.so: stringmap.o
	$(CC) -shared -o $@ stringmap.o

stringmaptest: stringmaptest.c
	$(CC) -g $(LIB_STRING_MAP_LIB) -o $@ $^

clean:
	rm -f lock.o
	rm -f stringmap.o
	rm -f stats.o
	rm -f shared.o
	rm -f psserver
	rm -f psclient

outs:
	rm *.stderr
	rm *.stdout
	



