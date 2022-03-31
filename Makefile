CC = gcc
CFLAGS = -g -Werror
LDFLAGS = -lpthread

proxy: proxy.c cache/cache.c cache/cache.h proxy-helpers/error_handling.c proxy-helpers/error_handling.h proxy-helpers/url_parser.c proxy-helpers/url_parser.h proxy-helpers/wrappers.c proxy-helpers/wrappers.h robust-input-output/robust_io.c robust-input-output/robust_io.h semaphore/semaphore.c semaphore/semaphore.h socket-interface/socket.c socket-interface/socket.h thread-helpers/sbuf.c thread-helpers/sbuf.h thread-helpers/thread.c thread-helpers/thread.h service.c service.h constants.h
	$(CC) $(CFLAGS) -o proxy service.c ./cache/cache.c ./proxy-helpers/error_handling.c ./robust-input-output/robust_io.c ./thread-helpers/sbuf.c ./semaphore/semaphore.c ./socket-interface/socket.c ./thread-helpers/thread.c ./proxy-helpers/url_parser.c ./proxy-helpers/wrappers.c proxy.c $(LDFLAGS)



clean:
	rm -f *~ *.o proxy

