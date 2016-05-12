
![](img.png)

----


# Awa LightweightM2M.

## Example: Create a client application on a gateway device using the Awa API.

### Application overview.

![](Awa_client_tutorial_application_positioning.png)

This example shows how to:

* Compile and install Awa LightweightM2M   
* Create the application *client-tutorial* which:
    * Initiates a client session
    * Defines an object
    * Defines a resource within the object
    * Instantiates the defined object within the client
    * closes the client session
    * Exits
* Enhance the initial application with the addition of another resource to the existing object
* Create a server side application which uses the same object model

The client-tutorial application makes use of the Awa API to define objects and resources and to register these with the client daemon:

![](client-tutorial.png)

Note that this example assumes you have downloaded and decompressed (or git-cloned) Awa LWM2M into the directory `~/AwaLWM2M`.

## Awa LightweightM2M installation.

Use the command below to build and install Awa LightweightM2M to the  *./build/install* directory:

```
~/AwaLWM2M $ make install DESTDIR=install
```

Alternatively, you can use the following command to install into the default directory on your system:

```
~/AwaLWM2M $ sudo make install
```

This example will assume that you have installed it to `~/AwaLWM2M/build/install`.

----


## Creating the application.


Now we create a new directory for the project. We'll use *tutorial*:

```
$ mkdir ~/tutorial
$ cd ~/tutorial
```

To create the makefile, copy the code below to tutorial/*Makefile*. Be sure to retain the ````<TAB>```` character preceding *$(CC) client-tutorial.c* :

```make
all:
	$(CC) client-tutorial.c -o client-tutorial -I$(AWA_INSTALL_PATH)/usr/include -L$(AWA_INSTALL_PATH)/usr/lib -lawa
```

Now is a good time to define our objects and resources:

![](Awa_client_tutorial_object_description.png)

To create the above object model the following code goes into tutorial/*client-tutorial.c*:

```c
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

Now build the application...

```
~/tutorial$ make AWA_INSTALL_PATH=~/AwaLWM2M/build/install
```

Start the client daemon...

````
~/AwaLWM2M$ build/core/src/client/awa_clientd --endPointName client1 --factoryBootstrap ./core/bootstrap-localhost.config --daemonise --logFile /tmp/awa_clientd.log
````

And run the application...

Note that in this case *libawa.so* isn't in the library path. We'll tell the system where to find
 it by setting the *LD_LIBRARY_PATH* variable.

```
~/tutorial$ LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib ./client-tutorial
````

The application will then exit, leaving the new object/resource registered within the client daemon.

Use the client tool *awa-client-get* to read your newly defined resource:

```
~/AwaLWM2M/build/install/bin$ ./awa-client-get /1000/0/101
Heater[/1000/0]:
    Manufacturer[/1000/0/101]: HotAir Systems Inc
```

----


## Adding further resources.

Further object resources are easily defined. Note that a resource's data type is implied by the API function used to create it, e.g. resource 101, (defined above), was of type string and thus used *AwaObjectDefinition_AddResourceDefinitionAsString()*. Our new resource, 104, will be of type float, so we'll use *AwaObjectDefinition_AddResourceDefinitionAsFloat()* to create it. Function parameters remain the same.

So our *client-tutorial.c* file now contains two additional lines:

In the *DefineHeaterObject()* function:

```c
AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature", false, AwaResourceOperations_ReadWrite, 0.0);
```

This defines the new resource.

And in the *SetInitialValues()* function:

```c
AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/104");
    AwaClientSetOperation_AddValueAsCString(operation, "/1000/0/101", "HotAir Systems Inc");
```

Which instantiates the resource.

We've also added a function called *UpdateTemperature()* which accepts a float value *temperature* and writes it to the new resource, and a *while* loop in the *main()* function which accepts temperature values entered at the console, (to simulate actual sensor values from a device for example), and uses *UpdateTemperature()* to update the resource to the entered value.

Here's the updated application code:

```c
static void DefineHeaterObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);
+   AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature", false, AwaResourceOperations_ReadWrite, 0.0);

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
+   AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/104");
    AwaClientSetOperation_AddValueAsCString(operation, "/1000/0/101", "HotAir Systems Inc");

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

+static void UpdateTemperature(AwaClientSession * session, float temperature)
+{    
+    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
+   
+    AwaClientSetOperation_AddValueAsFloat(operation, "/1000/0/104", temperature);
+
+    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
+    AwaClientSetOperation_Free(&operation);
+}

int main(void)
{
    AwaClientSession * session = AwaClientSession_New();

    AwaClientSession_Connect(session);

    DefineHeaterObject(session);
    SetInitialValues(session);

+   while (true)
+   {
+       float temperature; 
+
+       printf("enter temperature or any other key to exit:");
+       if (scanf("%f", &temperature) == 0)
+       {
+           break;
+       }
+
+       UpdateTemperature(session, temperature);
+       printf("set temperature /1000/0/104 to %f\n", temperature);
+   }

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}
```

After updating the application code, rebuild the application:

```
~/tutorial$ make AWA_INSTALL_PATH=~/AwaLWM2M/build/install
```

And restart the client daemon:

```
~/AwaLWM2M$ killall awa_clientd
~/AwaLWM2M$ build/core/src/client/awa_clientd --endPointName client1 --factoryBootstrap ./core/bootstrap-localhost.config --daemonise --logFile /tmp/awa_clientd.log
```

Then restart the client application and set the temperature...

```
~/tutorial$ LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib ./client-tutorial
enter temperature or any other key to exit:10.0
set temperature /1000/0/104 to 10.000000
enter temperature or any other key to exit:q
```

To check for success, use the client tools to read your newly defined resource:

```
~/AwaLWM2M/build/install/bin$ ./awa-client-get /1000/0/104
Heater[/1000/0]:
    Temperature[/1000/0/104]: 10
```

So far our object definitions have remained local to the client. The next section compliments our client application by extending our object definitions to the server.


----



## Creating a server application using the Awa API.

Let's create a server side application that makes use of our new object.

![](server-tutorial.png)

The following code registers our object definition with the LWM2M server daemon, allowing it to communicate with any LWM2M clients that support the same object.

Create the file tutorial/*server-tutorial.c* which contains the following code:

```c
#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/server.h>

#define OPERATION_PERFORM_TIMEOUT 1000

int main(void)
{
    AwaServerSession * session = AwaServerSession_New();

    AwaServerSession_Connect(session);

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);

    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature",  false, AwaResourceOperations_ReadWrite, 0.0);

    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);
    AwaServerDefineOperation_Add(operation, objectDefinition);
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaServerDefineOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}
```

Now update tutorial/Makefile to include *server-tutorial.c* like so:

```make
all:
        $(CC) client-tutorial.c -o client-tutorial -I$(AWA_INSTALL_PATH)/usr/include -L$(AWA_INSTALL_PATH)/usr/lib -lawa
        $(CC) server-tutorial.c -o server-tutorial -I$(AWA_INSTALL_PATH)/usr/include -L$(AWA_INSTALL_PATH)/usr/lib -lawa
```

Build the new application:

```
~/tutorial$ make AWA_INSTALL_PATH=~/AwaLWM2M/build/install
```

Restart the client/server daemon:

```
~/AwaLWM2M$ killall awa_serverd
~/AwaLWM2M$ killall awa_clientd
~/AwaLWM2M$ build/core/src/server/awa_serverd --verbose --daemonise --logFile /tmp/awa_serverd.log
~/AwaLWM2M$ build/core/src/client/awa_clientd --endPointName client1 --factoryBootstrap ./core/bootstrap-localhost.config --daemonise --logFile /tmp/awa_clientd.log
```

And start the server application:

```
~/tutorial$ LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib ./server-tutorial
```

Now start client client application:

```
~/tutorial$ LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib ./client-tutorial
```

Use the server tool *awa-server-list-clients* to check that the client is registered with the server (look for object /1000/0 ):

```
./awa-server-list-clients --objects
Client: client1
  /1/0     LWM2MServer
  /2/0     LWM2MAccessControl
  /2/1     LWM2MAccessControl
  /2/2     LWM2MAccessControl
  /2/3     LWM2MAccessControl
  /3/0     Device
  /4/0     ConnectivityMonitoring
  /7       ConnectivityStatistics
  /5/0     FirmwareUpdate
  /6/0     Location
  /1000/0  Heater
```

Read from the new resource using the server tool *awa-server-read*:

```
~/AwaLWM2M/build/install/bin$ ./awa-server-read -c client1 /1000/0/104
Heater[/1000/0]:
    Temperature[/1000/0/104]: 10
```

Alternatively you can read entire objects or object instances:

```
~/AwaLWM2M/build/install/bin$ ./awa-server-read -c client1 /1000
Heater[/1000/0]:
    Manufacturer[/1000/0/101]: HotAir Systems Inc
    Temperature[/1000/0/104]: 10

~/AwaLWM2M/build/install/bin$ ./awa-server-read -c client1 /1000/0
Heater[/1000/0]:
    Manufacturer[/1000/0/101]: HotAir Systems Inc
    Temperature[/1000/0/104]: 10
```

## Example: Create a standalone LWM2M client on a gateway device using the Awa Static API.

This example will demonstrate how to build a standalone LWM2M client using the Awa Static API.

Create a new directory *static-client-tutorial*.

Copy the following code into static-client-tutorial/*Makefile*:

```make
all:
	$(CC) static-client-tutorial.c -o static-client-tutorial -I$(AWA_INSTALL_PATH)/usr/include -L$(AWA_INSTALL_PATH)/usr/lib -lawa_static
```

Copy the following code into static-client-tutorial/*static-client-tutorial.c*:

```c
#include <string.h>
#include <stdio.h>
#include "awa/static.h"

int main(void)
{
    AwaStaticClient * awaClient = AwaStaticClient_New();

	AwaStaticClient_SetLogLevel(AwaLogLevel_Error);
    AwaStaticClient_SetEndPointName(awaClient, "AwaStaticClient1");
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", 6000);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coap://[127.0.0.1]:15685");

    AwaStaticClient_Init(awaClient);

    while (1)
    {
        AwaStaticClient_Process(awaClient);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}
```

Run "make" and specify the install path to Awa LWM2M:

```
$ cd static-client-tutorial
static-client-tutorial $ make AWA_INSTALL_PATH=~/AwaLWM2M/build/install
```

Start the bootstrap and server daemons:

```
$ ./build/install/bin/awa_bootstrapd -d --config core/bootstrap-localhost.config
$ ./build/install/bin/awa_serverd -d
```

Run your new application:

```
$ LD_LIBRARY_PATH=~/AwaLWM2M/build/install/usr/lib ./static-client-tutorial
```

Query the server for connected clients:

```
$ ./build/install/bin/awa-server-list-clients -o
Client: AwaStaticClient1
  /2/0     LWM2MAccessControl
  /2/1     LWM2MAccessControl
  /2/2     LWM2MAccessControl
  /2/3     LWM2MAccessControl
  /1/0     LWM2MServer
```

### Add a custom object using the Awa static API.

The following code expands on the previous example, by demonstrating how to add a custom object

```c
#include <string.h>
#include <stdio.h>
#include "awa/static.h"

+#define HEATER_INSTANCES 1

+typedef struct
+{
+    char Manufacturer[64];
+    AwaFloat Temperature;
+
+} HeaterObject;

+static HeaterObject heater[HEATER_INSTANCES];

+static void DefineHeaterObject(AwaStaticClient * awaClient)
+{
+    AwaStaticClient_DefineObject(awaClient, 1000, "Heater", 0, HEATER_INSTANCES);
+    AwaStaticClient_DefineResource(awaClient, 1000, 101, "Manufacturer", AwaResourceType_String, 0, 1, AwaResourceOperations_ReadOnly);
+    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 101, &heater[0].Manufacturer, sizeof(heater[0].Manufacturer), sizeof(heater[0]));
+    AwaStaticClient_DefineResource(awaClient, 1000, 104, "Temperature",  AwaResourceType_Float, 0, 1, AwaResourceOperations_ReadOnly);
+    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 104, &heater[0].Temperature, sizeof(heater[0].Temperature), sizeof(heater[0]));
+}

+static void SetInitialValues(AwaStaticClient * awaClient)
+{
+    int instance = 0;
+
+    AwaStaticClient_CreateObjectInstance(awaClient, 1000, instance);
+
+    AwaStaticClient_CreateResource(awaClient, 1000, instance, 101);
+    strcpy(heater[instance].Manufacturer, "HotAir Systems Inc");
+
+    AwaStaticClient_CreateResource(awaClient, 1000, instance, 104);
+    heater[instance].Temperature = 0.0;
+}

int main(void)
{
    AwaStaticClient * awaClient = AwaStaticClient_New();

    AwaStaticClient_SetLogLevel(AwaLogLevel_Error);
    AwaStaticClient_SetEndPointName(awaClient, "AwaStaticClient1");
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", 6000);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coap://[127.0.0.1]:15685");

    AwaStaticClient_Init(awaClient);

+   DefineHeaterObject(awaClient);
+   SetInitialValues(awaClient);

    while (1)
    {
        AwaStaticClient_Process(awaClient);

+       //heater[0].Temperature = value from hardware
+       AwaStaticClient_ResourceChanged(awaClient, 1000, 0, 104);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}

```

## Example: Create a standalone LWM2M client within a contiki environment.

Awa LWM2M includes a number of makefiles to allow it to be compiled out of tree 
for a contiki environment. 

The following instructions act as an example of how to build a LWM2M client for contiki.

*Note: In this case we will use the contiki simulated environment, hardware specific configuration
 is outside of the scope of this tutorial*

Create a new directory *contiki-example*:

```
$ mkdir contiki-example
```

Clone AwaLWM2M and contiki into this directory:

```
$ cd contiki-example
contiki-example$ git clone https://github.com/FlowM2M/AwaLWM2M.git 
contiki-example$ git clone https://github.com/contiki-os/contiki.git 

contiki-example$ ls
AwaLWM2M
contiki
```

Copy the following code into contiki-example/*Makefile*:

```make
CONTIKI_PROJECT=static-client-tutorial
CONTIKI=contiki
LWM2M_DIR=AwaLWM2M

CFLAGS += -Wall -Wno-pointer-sign
CFLAGS += -I$(LWM2M_DIR)/api/include -DLWM2M_CLIENT

CFLAGS += -DUIP_CONF_BUFFER_SIZE=4096
CFLAGS += -DREST_MAX_CHUNK_SIZE=512

APPS += er-coap
APPS += rest-engine

APPDIRS += $(LWM2M_DIR)/core/src
APPS += client
APPS += common

CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL = 0

all: static-client-tutorial

include $(CONTIKI)/Makefile.include
```

Copy the following code to contiki-example/*static-client-tutorial.c*:

```c
#include <stdio.h>
#include "contiki.h"
#include "awa/static.h"

#define HEATER_INSTANCES 1

typedef struct
{
    char Manufacturer[64];
    AwaFloat Temperature;

} HeaterObject;

static HeaterObject heater[HEATER_INSTANCES];

static void DefineHeaterObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_DefineObject(awaClient, 1000, "Heater", 0, HEATER_INSTANCES);
    AwaStaticClient_DefineResource(awaClient, 1000, 101, "Manufacturer", AwaResourceType_String, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 101, &heater[0].Manufacturer, sizeof(heater[0].Manufacturer), sizeof(heater[0]));
    AwaStaticClient_DefineResource(awaClient, 1000, 104, "Temperature", AwaResourceType_Float, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 104, &heater[0].Temperature, sizeof(heater[0].Temperature), sizeof(heater[0]));
}

static void SetInitialValues(AwaStaticClient * awaClient)
{
    int instance = 0;

    AwaStaticClient_CreateObjectInstance(awaClient, 1000, instance);

    AwaStaticClient_CreateResource(awaClient, 1000, instance, 101);
    strcpy(heater[instance].Manufacturer, "HotAir Systems Inc");

    AwaStaticClient_CreateResource(awaClient, 1000, instance, 104);
    heater[instance].Temperature = 0.0;
}

PROCESS(lwm2m_client, "Awa LWM2M Example Client");
AUTOSTART_PROCESSES(&lwm2m_client);

PROCESS_THREAD(lwm2m_client, ev, data)
{
    PROCESS_BEGIN();

    static AwaStaticClient * awaClient;

    awaClient = AwaStaticClient_New();

	AwaStaticClient_SetLogLevel(AwaLogLevel_Error);
    AwaStaticClient_SetEndPointName(awaClient, "AwaStaticClient1");
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "", 6000);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coap://[fe80::1]:15683");

    AwaStaticClient_Init(awaClient);

    DefineHeaterObject(awaClient);
    SetInitialValues(awaClient);

    while (1)
    {
        static struct etimer et;
        static int waitTime;

        waitTime = AwaStaticClient_Process(awaClient);

        //heater[0].Temperature = value from hardware
        AwaStaticClient_ResourceChanged(awaClient, 1000, 0, 104);

        etimer_set(&et, (waitTime * CLOCK_SECOND) / 1000);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        waitTime = 0;
    }

    AwaStaticClient_Free(&awaClient);

    PROCESS_END();
}
```

Build your contiki application:

```
contiki-example$ make TARGET=minimal-net
```

----
----
