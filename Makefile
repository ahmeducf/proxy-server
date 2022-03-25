# Makefile for Proxy Lab 


CC = gcc
CFLAGS = -g -Werror
LDFLAGS = -lpthread

proxy: proxy.c cache/cache.c proxy-helpers service.c robust-input-output/robust_io.c thread-helpers semaphore/semaphore.c socket-interface/socket.c thread-helpers proxy-helpers proxy-helpers cache/cache.h constants.h proxy-helpers service.h robust-input-output/robust_io.h thread-helpers semaphore/semaphore.h socket-interface/socket.h thread-helpers proxy-helpers proxy-helpers
	$(CC) $(CFLAGS) -o proxy service.c ./cache/cache.c ./proxy-helpers/error_handling.c ./robust-input-output/robust_io.c ./thread-helpers/sbuf.c ./semaphore/semaphore.c ./socket-interface/socket.c ./thread-helpers/thread.c ./proxy-helpers/url_parser.c ./proxy-helpers/wrappers.c proxy.c $(LDFLAGS)


# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!


clean:
	rm -f *~ *.o proxy

