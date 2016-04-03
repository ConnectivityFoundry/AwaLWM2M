INSTALL_PATH:=../../../build/install

all:
	$(CC) client-tutorial1.c -o client-tutorial1 -I$(INSTALL_PATH)/usr/include -L$(INSTALL_PATH)/usr/lib -lawa
	$(CC) client-tutorial2.c -o client-tutorial2 -I$(INSTALL_PATH)/usr/include -L$(INSTALL_PATH)/usr/lib -lawa
	$(CC) server-tutorial.c -o server-tutorial -I$(INSTALL_PATH)/usr/include -L$(INSTALL_PATH)/usr/lib -lawa
	$(CC) static-client-tutorial1.c -o static-client-tutorial1 -I$(INSTALL_PATH)/usr/include -L$(INSTALL_PATH)/usr/lib -lawa_static
	$(CC) static-client-tutorial2.c -o static-client-tutorial2 -I$(INSTALL_PATH)/usr/include -L$(INSTALL_PATH)/usr/lib -lawa_static
