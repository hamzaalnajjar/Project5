# CMSC216 Project 5 Makefile
AN = p5
CLASS = 216

# -Wno-unused : don't warn for unused vars
# Provide debug level optimizations
CFLAGS = -Wall -Wno-comment -Werror -g  -Og
CC     = gcc $(CFLAGS)
SHELL  = /bin/bash
CWD    = $(shell pwd | sed 's/.*\///g')

PROGRAMS = \
	el_malloc.o \
	el_demo \
	test_el_malloc \
	sumdiag_print \
	sumdiag_benchmark \

all : $(PROGRAMS)

clean :
	rm -f $(PROGRAMS) *.o vgcore.* 

############################################################
# help message to show build targets
help :
	@echo 'Typical usage is:'
	@echo '  > make                          # build all programs'
	@echo '  > make clean                    # remove all compiled items'
	@echo '  > make zip                      # create a zip file for submission'
	@echo '  > make prob1                    # built targets associated with problem 1'
	@echo '  > make prob1 testnum=5          # run problem 1 test #5 only'
	@echo '  > make test-prob2               # run test for problem 2'
	@echo '  > make test                     # run all tests'

############################################################
# 'make zip' to create complete.zip for submission
ZIPNAME = $(AN)-complete.zip
zip : clean clean-tests
	rm -f $(ZIPNAME)
	cd .. && zip "$(CWD)/$(ZIPNAME)" -r "$(CWD)"
	@echo Zip created in $(ZIPNAME)
	@if (( $$(stat -c '%s' $(ZIPNAME)) > 10*(2**20) )); then echo "WARNING: $(ZIPNAME) seems REALLY big, check there are no abnormally large test files"; du -h $(ZIPNAME); fi
	@if (( $$(unzip -t $(ZIPNAME) | wc -l) > 256 )); then echo "WARNING: $(ZIPNAME) has 256 or more files in it which may cause submission problems"; fi

################################################################################
# build .o files from corresponding .c files
%.o : %.c
	$(CC) -c $<


################################################################################
# EL MALLOC
el_malloc.o : el_malloc.c el_malloc.h
	$(CC) -c $<

el_demo : el_demo.c el_malloc.o
	$(CC) -o $@ $^

test_el_malloc : test_el_malloc.c el_malloc.o
	$(CC) -o $@ $^

################################################################################
# Matrix diagonal summing optimization problem
sumdiag_print : sumdiag_print.o sumdiag_util.o sumdiag_base.o sumdiag_optm.o
	$(CC) -o $@ $^ -lm -lpthread

sumdiag_benchmark : sumdiag_benchmark.o sumdiag_util.o sumdiag_base.o sumdiag_optm.o
	$(CC) -o $@ $^ -lm -lpthread


################################################################################
# Testing Targets
test: test-prob1 test-prob2

test-setup :
	@chmod u+rx testy

test-prob1: el_demo test_el_malloc test-setup el_demo
	./testy test_el_malloc.org $(testnum)

test-prob2: sumdiag_benchmark sumdiag_print test-setup
	./testy test_sumdiag.org $(testnum)

clean-tests :
	rm -rf test-results


