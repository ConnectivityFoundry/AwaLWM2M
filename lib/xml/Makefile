CC=gcc
AR=ar
_STAGING_DIR:=../../staging

all: compile install

install: compile
	mkdir -p $(_STAGING_DIR)/usr/lib
	cp libxml.so $(_STAGING_DIR)/usr/lib

compile:
	$(CC) -fPIC -o xmltree.o -c xmltree.c $(CFLAGS)
	$(CC) -fPIC -o xmlparser.o -c xmlparser.c $(CFLAGS)
	$(AR) rcs libxml.a xmlparser.o xmltree.o
	$(LD) -shared -o libxml.so xmlparser.o xmltree.o

clean:
	rm -f *.o *.a *.so
