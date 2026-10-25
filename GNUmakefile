## file boehm-rps/GNUmakefile
## see https://github.com/bstarynk/boehm-rps
## Author(s):
##      Basile STARYNKEVITCH, 92340 Bourg-la-Reine, France,
##                    <basile@starynkevitch.net>
CC = gcc
INDENT ?= indent
OPTIM ?= -O1
CFLAGS += -Wall -Wextra -Wmissing-prototypes -std=gnu17 $(OPTIM) -g
CFLAGS += $(shell pkg-config --cflags bdw-gc)
LDFLAGS += $(shell pkg-config --libs bdw-gc) 
HUMAN_HEADER= boehmrps.h
HUMAN_C_FILES = $(wildcard [a-z]*.c)
HUMAN_OBJ_FILES = $(patsubst %.c, obj/%.o, $(HUMAN_C_FILES))
### convention generated/_*_.c are temporary
GENERATED_C_FILES = $(wildcard generated/*.c)
.SUFFIXES: .i 
.PHONY: all clean indent
all: boehm-rps

clean:
	/bin/rm -vf *~ obj/*.o boehm-rps _*_.c *.i

indent:
	$(INDENT) -v $(HUMAN_HEADER)
	for f in $(HUMAN_C_FILES) ; do  $(INDENT) -v $$f; done

obj/%.o: %.c $(HUMAN_HEADER) | GNUmakefile get-short-git.sh
	$(CC) $(CFLAGS) -DBPRS_SHORTGIT=\"$(shell ./get-short-git.sh $^)\" -c $< -o $@

%.i: %.c $(HUMAN_HEADER)  | GNUmakefile get-short-git.sh
	$(CC) $(CFLAGS) -DBPRS_SHORTGIT=\"$(shell ./get-short-git.sh $^)\" -C -E $< | sed 's:^#://#:g' | $(INDENT) > $@

boehm-rps: $(HUMAN_OBJ_FILES) obj/_list_.o | $(HUMAN_HEADER) GNUmakefile
	$(CC) -ULINKBOEHMRPS $(CFLAGS)  $^ \
               $(LDFLAGS) -rdynamic -lunistring -lm -ldl -o $@
	/bin/mv -vf _list_.c _list_.c~

_list_.c: GNUmakefile
	printf "const char bprs_shortgit[] = \"%s\";\n" \
            $(shell ./get-short-git.sh .) > $@
	printf "const char bprs_sourcedir[] = \"%s\";\n" $(shell realpath $$PWD) >> $@
	printf "const char* bprs_human_files[]= {\n" >> $@
	for f in $(HUMAN_C_FILES); do printf "  \"%s\",\n" $$f; done >> $@
	echo "  (const char*)0 };\n" >> $@
	printf "const char* bprs_human_md5[]= {\n" >> $@
	for f in $(HUMAN_C_FILES); do printf "  \"%s\", // %s\n" $$(md5sum $$f </dev/null); done >> $@
	echo "  (const char*)0 };\n" >> $@
	printf "const char* bprs_generated_files[]= {\n" >> $@
	for f in $(GENERATED_C_FILES); do printf "  \"%s\",\n" $$f; done >> $@
	echo "  (const char*)0 };\n" >> $@
	printf "const char* bprs_generated_md5[]= {\n" >> $@
	for f in $(GENERATED_C_FILES); do printf "  \"%s\",\n" $$(md5sum $$f</dev/null); done >> $@
	echo "  (const char*)0 };\n" >> $@

