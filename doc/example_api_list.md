
![](images/img.png)

----
## [client-defineset-example](../api/examples/client-defineset-example.c)

 This example demonstrates how to define a new object definition, consisting
 of a number of resources, on the client using a DEFINE operation.


## [client-definition-iterator-example](../api/examples/client-definition-iterator-example.c)

 This example demonstrates how the AwaObjectDefinitionIterator and
 AwaResourceDefinitionIterator can be used to iterate through all of
 the objects and resources that have been previously defined in a client session.


## [client-delete-array-range-example](../api/examples/client-delete-array-range-example.c)

 This example demonstrates how to delete a range of resource instances from
 a multi-instance resource (array) on the client using a DELETE operation.


## [client-delete-example](../api/examples/client-delete-example.c)

 This example demonstrates how to delete a resource from the client using a DELETE operation.


## [client-get-array-example](../api/examples/client-get-array-example.c)

 This example demonstrates how to retrieve the value of a resource instance
 from a multi-instance resource (array) on the client using a GET operation.


## [client-get-contains-path-example](../api/examples/client-get-contains-path-example.c)

 This example demonstrates how to test whether a GET operation response contains
 a specific path. It also demonstrates retrieval of a value from a string resource.


## [client-get-example](../api/examples/client-get-example.c)

 This example demonstrates how to retrieve the value of a resource from the client
 using a GET operation.


## [client-set-array-example](../api/examples/client-set-array-example.c)

 This example demonstrates how to set the value of a resource instance of a
 multi-instance resource on the client using the SET operation.


## [client-set-create-example](../api/examples/client-set-create-example.c)

 This example demonstrates how to create and set the value of an optional resource
 on the client using the SET operation.


## [client-set-example](../api/examples/client-set-example.c)

 This example demonstrates how to set the value of a resource on the client
 using the SET operation.


## [client-subscribe-to-change-example](../api/examples/client-subscribe-to-change-example.c)

 This example demonstrates how to subscribe to the client for notifications of
 changes to the value of a resource, using a SUBSCRIBE to CHANGE operation.

 This example is a little contrived in that it creates a subscription and then
 triggers the notification itself. In most cases, the notification will be
 triggered by a WRITE operation initiated by the server.


## [client-subscribe-to-execute-example](../api/examples/client-subscribe-to-execute-example.c)

 This example demonstrates how to subscribe to the client for notifications of
 execution of a resource, using a SUBSCRIBE to EXECUTE operation.

 This example is a little contrived in that it creates a subscription and then
 triggers the notification itself. In most cases, the notification will be
 triggered by an EXECUTE operation initiated by the server.


## [server-defineset-example](../api/examples/server-defineset-example.c)

 This example demonstrates how to define a new object definition, consisting
 of a number of resources, on the server using a DEFINE operation.


## [server-delete-object-instance-example](../api/examples/server-delete-object-instance-example.c)

 This example demonstrates how to delete an object instance from
 the server using the DELETE operation.


## [server-event-example](../api/examples/server-event-example.c)

 This example demonstrates how to respond to client events from the server.

 If a client registers, deregisters or updates its registration with the server,
 an application callback is invoked.

 To run this example, run an Awa server and then run this example. Then
 start a LWM2M client (such as the Awa client) and have it register with this
 server. Hit CTRL-C to terminate.


## [server-execute-arguments-example](../api/examples/server-execute-arguments-example.c)

 This example demonstrates how to initiate an EXECUTE operation via the server,
 passing application-specific data to the executable resource on the client.


## [server-execute-example](../api/examples/server-execute-example.c)

 This example demonstrates how to initiate an EXECUTE operation via the server,
 targeting an executable resource on the client.


## [server-list-clients-example](../api/examples/server-list-clients-example.c)

 This example demonstrates how to obtain a list of all registered clients from
 the server using a LIST_CLIENTS operation. In addition, the list of registered
 objects and object instances is extracted from the response.


## [server-observe-example](../api/examples/server-observe-example.c)

 This example demonstrates how to observe a client's resource via the server.
 If the resource changes and a notification is generated, an application callback
 is invoked.

 This example is a little contrived in that it creates an observation and then
 triggers the notification itself with a WRITE. In many cases, the notification
 will be triggered by an update to the resource value by another entity such as
 the client itself.


## [server-read-example](../api/examples/server-read-example.c)

 This example demonstrates how to read a resource value from a registered
 client with a READ operation.


## [server-write-create-example](../api/examples/server-write-create-example.c)

 This example demonstrates how to create a new object instance on a
 registered client with a WRITE operation. In this case, the object
 instance ID is specified by the application.


## [server-write-create-unspecified-instance-example](../api/examples/server-write-create-unspecified-instance-example.c)

 This example demonstrates how to create a new object instance on a
 registered client with a WRITE operation. In this case, the object
 instance ID is not specified by the application, and chosen by the
 client.


## [server-write-example](../api/examples/server-write-example.c)

 This example demonstrates how to set the value of a resource on a registered
 client with a WRITE operation.


----
----
