## Writing your own client application using the Awa API

Before we can start Awa LWM2M must be compiled and installed,
the commands below can be used to install Awa LWM2M to ./build/install

```
~/AwaLWM2M$ make
~/AwaLWM2M$ cd build
~/AwaLWM2M/build$ cmake DESTDIR=./install install
```

Create a new directory for your project, in this case we will use "client-tutorial"

```
$ mkdir ~/client-tutorial
$ cd ~/client-tutorial
```

Copy the code below to client-tutorial/Makefile, making sure to retain the <TAB> character
preceeding $(CC) client-tutorial.c 

```
INSTALL_PATH:=~/AwaLWM2M/build/install

all:
	$(CC) client-tutorial.c -o client-tutorial -I $(INSTALL_PATH)/usr/include -L $(INSTALL_PATH)/usr/lib -lawa
```

Copy the code below to client-tutorial/client-tutorial.c

```
#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#define OPERATION_PERFORM_TIMEOUT 1000

static void DefineHeaterObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

static void SetInitialValues(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_CreateObjectInstance(operation, "/1000/0");
    AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/101");
    AwaClientSetOperation_AddValueAsCString(operation, "/1000/0/101", "HotAir Systems Inc");

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

int main(void)
{
    AwaClientSession * session = AwaClientSession_New();

    AwaClientSession_Connect(session);

    DefineHeaterObject(session);
    SetInitialValues(session);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}
```

Build your new application 

```
~/client-tutorial$ make INSTALLPATH=~/AwaLWM2M/build/install
```

Start the server and client

````
~/AwaLWM2M$ build/core/src/server/awa_serverd --verbose --daemonise --logFile /tmp/awa_serverd.log
~/AwaLWM2M$ build/core/src/client/awa_clientd --endPointName client1 --factoryBootstrap ./core/bootstrap-localhost.config --daemonise --logFile /tmp/awa_clientd.log
````

Run your application
```
~/client-tutorial$ export LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib; ./client-tutorial
````

At this point your application will exit, leaving your new object/resource registered within the client daemon.

Use the server tools to read your newly defined resource

```
~/AwaLWM2M/build/install$ ./awa-server-read -c client1 /1000/101
```

