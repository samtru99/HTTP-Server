CXX = clang

CFLAGS = -Wall -Werror -Wextra -pedantic -std=c17 -g

httpserver: httpserver.o bind.o extract_header.o request_regex.o verify_header.o put.o get.o head.o audit.o
	$(CXX) $(CFLAGS) -o httpserver httpserver.o bind.o extract_header.o request_regex.o verify_header.o put.o get.o head.o audit.o -lm

httpserver.o: httpserver.c 
	$(CXX) $(CFLAGS) -c httpserver.c

bind.o: bind.c
	$(CXX) $(CFLAGS) -c bind.c

extract_header.o: extract_header.c
	$(CXX) $(CFLAGS) -c extract_header.c

request_regex.o: request_regex.c
	$(CXX) $(CFLAGS) -c request_regex.c

verify_header.o: verify_header.c
	$(CXX) $(CFLAGS) -c verify_header.c 

put.o: put.c
	$(CXX) $(CFLAGS) -c put.c

get.o: get.c
	$(CXX) $(CFLAGS) -c get.c

head.o: head.c
	$(CXX) $(CFLAGS) -c head.c

audit.o: audit.c
	$(CXX) $(CFLAGS) -c audit.c

clean:
	rm *.o httpserver httpserver.o
