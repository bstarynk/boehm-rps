## file boehm-rps/GNUmakefile
## see https://github.com/bstarynk/boehm-rps
## Author(s):
##      Basile STARYNKEVITCH, 92340 Bourg-la-Reine, France,
##                    <basile@starynkevitch.net>
CC ?= gcc
INDENT ?= indent
CFLAGS += -Wall -Wextra -O -g
CFLAGS += $(shell pkg-config --cflags bdw-gc)
LDFLAGS += $(shell pkg-config --libs bdw-gc) 
HUMAN_HEADER= boehmrps.h
HUMAN_C_FILES = $(wildcard [a-z]*.c)
HUMAN_OBJ_FILES = $(patsubst %.c, obj/%.o, $(HUMAN_C_FILES))
### convention generated/_*_.c are temporary
GENERATED_C_FILES = $(wildcard generated/*.c)
.PHONY: all clean indent
all: boehm-rps

clean:
	/bin/rm -vf *~ obj/*.o boehm-rps

indent:
	$(INDENT) -v $(HUMAN_HEADER)
	for f in $(HUMAN_C_FILES) ; do  $(INDENT) -v $$f; done

obj/%.o: %.c $(HUMAN_HEADER) | GNUmakefile
	$(CC) $(CFLAGS) -c $< -o $@

boehm-rps: $(HUMAN_OBJ_FILES) | $(HUMAN_HEADER) GNUmakefile
	$(CC) -ULINKBOEHMRPS $(CFLAGS) $(HUMAN_OBJ_FILES) \
               $(LDFLAGS) -rdynamic -o $@

