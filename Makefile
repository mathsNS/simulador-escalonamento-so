CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude -Isrc
LDLIBS ?= -lm

COMMON = src/simulator.c src/scheduler_fcfs.c src/scheduler_round_robin.c src/scheduler_priority.c src/scheduler_epa.c

.PHONY: all test clean
all: simulator

simulator: src/main.c $(COMMON) include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ src/main.c $(COMMON)

test_schedulers: tests/test_schedulers.c $(COMMON) include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_schedulers.c $(COMMON)

test_workload: tests/test_workload.c src/workload.c $(COMMON) include/workload.h include/simulator.h include/scheduler_internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_workload.c src/workload.c $(COMMON) $(LDLIBS)

test_scenarios: tests/test_scenarios.c src/scenarios.c src/workload.c include/scenarios.h include/workload.h include/simulator.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_scenarios.c src/scenarios.c src/workload.c $(LDLIBS)

test: test_schedulers test_workload test_scenarios
	./test_schedulers
	./test_workload
	./test_scenarios

clean:
	$(RM) simulator test_schedulers test_workload test_scenarios \
	      simulator.exe test_schedulers.exe test_workload.exe test_scenarios.exe
