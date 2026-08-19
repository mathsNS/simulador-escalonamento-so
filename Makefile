CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude -Isrc

COMMON = src/simulator.c src/scheduler_fcfs.c src/scheduler_round_robin.c src/scheduler_priority.c src/scheduler_epa.c

.PHONY: all test clean
all: simulator

simulator: src/main.c $(COMMON) include/simulator.h src/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ src/main.c $(COMMON)

test_schedulers: tests/test_schedulers.c $(COMMON) include/simulator.h src/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_schedulers.c $(COMMON)

test: test_schedulers
	./test_schedulers

clean:
	$(RM) simulator test_schedulers simulator.exe test_schedulers.exe
