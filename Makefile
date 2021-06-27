#
# Makefile
#
# Operating Systems, Exercise 4
#

CC = gcc
#CFLAGS = -Wall -g
CFLAGS = -Wall -O2 -g

all:  shell prog scheduler-priority

scheduler-priority: scheduler-priority.o proc-common.o queue2.o
	$(CC) -o scheduler-priority scheduler-priority.o proc-common.o queue2.o

shell: shell.o proc-common.o
	$(CC) -o shell shell.o proc-common.o

prog: prog.o proc-common.o
	$(CC) -o prog prog.o proc-common.o

proc-common.o: proc-common.c proc-common.h
	$(CC) $(CFLAGS) -o proc-common.o -c proc-common.c

shell.o: shell.c proc-common.h request.h
	$(CC) $(CFLAGS) -o shell.o -c shell.c

scheduler-priority.o: scheduler-priority.c proc-common.h request.h
	$(CC) $(CFLAGS) -o scheduler-priority.o -c scheduler-priority.c

queue2.o: queue2.c
	$(CC) $(FLAGS) -o queue2.o -c queue2.c

prog.o: prog.c
	$(CC) $(CFLAGS) -o prog.o -c prog.c
	
clean:
	rm -f scheduler scheduler-shell shell prog execve-example strace-test sigchld-example *.o
