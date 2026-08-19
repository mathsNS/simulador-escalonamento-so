CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude -Isrc
LDLIBS ?= -lm

COMMON = src/simulator.c src/scheduler_fcfs.c src/scheduler_round_robin.c src/scheduler_priority.c

.PHONY: all test clean
all: simulator

simulator: src/main.c $(COMMON) include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ src/main.c $(COMMON)

test_schedulers: tests/test_schedulers.c $(COMMON) include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_schedulers.c $(COMMON)

test_workload: tests/test_workload.c src/workload.c $(COMMON) include/workload.h include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_workload.c src/workload.c $(COMMON) $(LDLIBS)

test: test_schedulers test_workload
	./test_schedulers
	./test_workload

clean:
	$(RM) simulator test_schedulers test_workload simulator.exe test_schedulers.exe test_workload.exe
