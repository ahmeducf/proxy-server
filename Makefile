# Makefile for Proxy Lab 


CC = gcc
CFLAGS = -g -Werror
LDFLAGS = -lpthread

proxy: proxy.c cache.c ./proxy_helpers/error_handling.c service.c robust_io.c ./thread_helpers/sbuf.c ./thread_helpers/semaphore.c socket.c thread.c ./proxy_helpers/url_parser.c ./proxy_helpers/wrappers.c cache.h constants.h ./proxy_helpers/error_handling.h service.h robust_io.h ./thread_helpers/sbuf.h ./thread_helpers/semaphore.h socket.h thread.h ./proxy_helpers/url_parser.h ./proxy_helpers/wrappers.h
	$(CC) $(CFLAGS) -o proxy service.c cache.c ./proxy_helpers/error_handling.c robust_io.c ./thread_helpers/sbuf.c ./thread_helpers/semaphore.c socket.c thread.c ./proxy_helpers/url_parser.c ./proxy_helpers/wrappers.c proxy.c $(LDFLAGS)


# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!


clean:
	rm -f *~ *.o proxy

