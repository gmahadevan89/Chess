all: test chess_scheduler

chess_scheduler: scheduler.o
	gcc -g -o $@ $^

scheduler.o: scheduler.c
	gcc -g -c -o $@ $^

test: libmypthreadwrapper.so $(TEST)
	gcc -g -o $@ $^ -pthread -lmypthreadwrapper -L./ -Wl,-R./ 	

libmypthreadwrapper.so: mutex_helper.c mypthreadwrapper.c
	gcc -g -shared -o $@ -D_GNU_SOURCE -fPIC $^ -lpthread -ldl 

.PHONY: clean clean_r

clean: 
	rm -rf test chess_scheduler *.so *.o trace replay errs/

clean_r:
	rm -rf trace replay errs/
