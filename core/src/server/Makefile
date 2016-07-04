FILES:=awa_serverd_cmdline

# prevent autodeletion of intermediate files:
#.SECONDARY:

all: $(FILES:=.c)

.PHONY: gen_cmdline
gen_cmdline: $(FILES)

%_cmdline.c %_cmdline.h: %.ggo
	gengetopt --input=$^ --include-getopt --unamed-opts=PATHS --file $*_cmdline

.PHONY: clean
clean:
	rm -f $(FILES:=.c) $(FILES:=.h)
