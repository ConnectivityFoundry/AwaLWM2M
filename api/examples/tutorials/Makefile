# This makefile assumes that Awa has been built with the default INSTALL_PREFIX and DESTDIR=install.
AWA_INSTALL_PATH:=../../../build/install

all:
	$(CC) client-tutorial1.c -o client-tutorial1 -I$(AWA_INSTALL_PATH)/include -L$(AWA_INSTALL_PATH)/lib -lawa
	$(CC) client-tutorial2.c -o client-tutorial2 -I$(AWA_INSTALL_PATH)/include -L$(AWA_INSTALL_PATH)/lib -lawa
	$(CC) server-tutorial.c -o server-tutorial -I$(AWA_INSTALL_PATH)/include -L$(AWA_INSTALL_PATH)/lib -lawa
	$(CC) static-client-tutorial1.c -o static-client-tutorial1 -I$(AWA_INSTALL_PATH)/include -L$(AWA_INSTALL_PATH)/lib -lawa_static
	$(CC) static-client-tutorial2.c -o static-client-tutorial2 -I$(AWA_INSTALL_PATH)/include -L$(AWA_INSTALL_PATH)/lib -lawa_static
