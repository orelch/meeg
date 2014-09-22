############################################################################
#                                                                          #
#  Copyright (C) 2014 Utix                                                 #
#                                                                          #
#                                                                          #
#  Should you receive a copy of this source code, you must check you       #
#  have a proper, written authorization of Utix to hold it. If you         #
#  don't have such an authorization, you must DELETE all source code       #
#  files in your possession, and inform Utix of the fact you obtain        #
#  these files. Should you not comply to these terms, you can be           #
#  prosecuted in the extent permitted by applicable law.                   #
#                                                                          #
#   contact@utix.fr                                                        #
#                                                                          #
############################################################################
CC=gcc
CFLAGS := $(filter-out -O%,$(CFLAGS))
CFLAGS += -O0 -Wno-uninitialized -fno-inline -fno-inline-functions
CFLAGS += -g3 -std=gnu99


.SECONDEXPANSION:
TARGETS = edf

edf_SOURCES = main.c edf.c

all: $(TARGETS)

edf: $$($$@_SOURCES)
	$(CC)  -o $@ $(CFLAGS) $^


clean:
	rm -rf $(TARGETS)

ignore:
	echo $(TARGETS) > .gitignore
