FILES:=main_cmdline

all: $(FILES:=.c)

.PHONY: gen_cmdline
gen_cmdline: $(FILES)

%_cmdline.c %_cmdline.h: %.ggo
	gengetopt --input=$^ --include-getopt --file $*_cmdline

.PHONY: clean
clean:
	rm -f $(FILES:=.c) $(FILES:=.h)


