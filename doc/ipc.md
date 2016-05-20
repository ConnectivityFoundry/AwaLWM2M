# Introduction

The XML Interface is used to provide a language-independent interface for the LWM2M Client and Server daemons.

Requests consist of XML documents that are sent via the IPC channel from the application to the daemon.

Responses are returned as XML documents. Users of the IPC interface are referred to as IPC Clients.

String content is BASE-64 encoded when it appears within the XML.

# Common Operations

## Invalid Request

Any requests that are unrecognised or malformed will generate a 400 error response:

```xml
<Response>
  <Type>Invalid</Type>
  <Code>400</Code>
</Response>
```

## Connect

Initiates a session between the IPC client and the daemon. A session ID is returned which must be used in all session requests. Definitions for all defined objects are returned.

```xml
<Request>
  <Type>Connect</Type>
  <Target>Client</Target>  <!-- optional IPC target -->
</Request>
```

Later, we can add an API version and/or build identifier to warn users of out-of-date tools/daemon.

The response contains object definitions for all objects known by the daemon.

```xml
<Response>
  <Type>Connect</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <ObjectDefinitions>
      <ObjectDefinition>
        <ID>3</ID>
        <SerialisationName>Device</SerialisationName>
        <MinimumInstances>0</MinimumInstances>
        <MaximumInstances>1</MaximumInstances>
        <ResourceDefinitions>
          <ResourceDefinition>
            <ID>0</ID>
            <DataType>String</DataType>
            <IsCollection>False</IsCollection>
            <IsMandatory>True</IsMandatory>
            <Access>Read</Access>
            <SerialisationName>Manufacturer</SerialisationName>
          </ResourceDefinition>
          <!-- ... -->
        </ResourceDefinitions>
      </ObjectDefinition>
      <!-- ... -->
    </ObjectDefinitions>
  </Content>
</Response>
```

## EstablishNotify

Initiates a notification session between the IPC client and the daemon. This allows the daemon to store the Notification channel socket for use when sending Observe notifications and Events.

```xml
<Request>
  <Type>ConnectNotify</Type>
  <SessionID>12345678</SessionID>
  <Target>Client</Target>  <!-- optional IPC target -->
</Request>
```

The response contains a response code and no content.

```xml
<Response>
  <Type>ConnectNotify</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
</Response>
```

## Disconnect

Terminates a session between the IPC client and the daemon.

```xml
<Request>
  <Type>Disconnect</Type>
  <SessionID>12345678</SessionID>
</Request>
```

```xml
<Response>
  <Type>Disconnect</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
</Response>
```

## Define

Define requests the definition of object metadata. It replaces use 1 of the Register message.

```xml
<Request>
  <Type>Define</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <ObjectDefinitions>
      <ObjectDefinition>
        <ID>1001</ID>
        <Name>Test Object</Name>
        <MIMEType>application/vnd.oma.lwm2m.testobject</MIMEType>
        <SerialisationName>TestObject</SerialisationName>
        <MaximumInstances>1</MaximumInstances>
        <MinimumInstances>0</MinimumInstances>
        <ResourceDefinitions>
          <ResourceDefinition>
            <ID>0</ID>
            <Name>Test Resource</Name>
            <DataType>String</DataType>
            <DataTypeLength>255</DataTypeLength>
            <IsCollection>False</IsCollection>
            <IsMandatory>True</IsMandatory>
            <SerialisationName>Test Resource</SerialisationName>
          </ResourceDefinition>
          <!-- ... -->
        </ResourceDefinitions>
      </ObjectDefinition>
      <!-- ... -->
    </ObjectDefinitions>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Define</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <ObjectDefinitions>
      <ObjectDefinition>
        <ID>0</ID>
        <Defined/>
        <ResourceDefinitions>
          <ResourceDefinition>
            <ID>0</ID>
            <Defined/>
          </ResourceDefinition>
          <ResourceDefinition>
            <ID>1</ID>
            <Defined/>
          </ResourceDefinition>
        </ResourceDefinitions>
      </ObjectDefinition>
      <!-- ... -->
    </ObjectDefinitions>
  </Content>
</Response>
```

Other per-resource status codes could include:

```xml
<AlreadyExists/>   <!-- implies it hasn't been changed -->
<Invalid/>
```

# LWM2M Client Daemon

The LWM2M Client Daemon hosts a number of LWM2M resources. XML Interface messages are provided for the purposes of an application to create, delete, read and write these resources.

The following messages are used by the FlowM2M API to communicate with the LWM2M Client Daemon.

## Get

Enables an IPC client to retrieve the current value of a resource, or the values of all resources in an object instance, or the values of all resources of all instances of an object. Array-type resources are retrieved in their entirety.

```xml
<Request>
  <Type>Get</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>             <!-- get single resource value, including all resource instances -->
          </Resource>
        </ObjectInstance>
        <ObjectInstance>
          <ID>1</ID>
          <Resource>
            <ID>7</ID>
            <ResourceInstance>     <!-- for single resource instances -->
              <ID>17</ID>
            </ResourceInstance>
            <ResourceInstance>     <!-- for range of resource instances -->
              <IDRange>
                <Start>0</Start>
                <EndExclusive>4</EndExclusive>
              </IDRange>
            </ResourceInstance>
            <ResourceInstance>     <!-- for a count of resource instances -->
              <IDCount>
                <Start>100</Start>
                <Count>20</Count>
              </IDCount>
            </ResourceInstance>
          </Resource>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>4</ID>     <!-- get all instances of this object -->
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: the response contains the values of all non-executable resources specified in the request.

```xml
<Response>
  <Type>Get</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <Value>VALUE</Value>
            <Result>
			  <Error>Success</Error>
		    </Result>
          </Resource>
        </ObjectInstance>
        <ObjectInstance>
          <ID>1</ID>
          <Resource>
            <ID>7</ID>
            <ResourceInstance>
              <ID>0</ID>
              <Value>VALUE</Value>
              <Result>
			    <Error>Success</Error>
              </Result>
            </ResourceInstance>
            <ResourceInstance>
              <ID>1</ID>
              <Value>VALUE</Value>
              <Result>
                <Error>Success</Error>
              </Result>
            </ResourceInstance>
            <ResourceInstance>
              <ID>2</ID>
              <Value>VALUE</Value>
              <Result>
                <Error>Success</Error>
              </Result>
            </ResourceInstance>
            <ResourceInstance>
              <ID>3</ID>
              <Value>VALUE</Value>
              <Result>
                <Error>Success</Error>
              </Result>
            </ResourceInstance>
            <ResourceInstance>
              <ID>17</ID>
              <Value>VALUE</Value>
              <Result>
                <Error>Success</Error>
              </Result>
            </ResourceInstance>
          </Resource>
        </Instance>
      </Object>
    </Objects>
  </Content>
</Response>
```

## Set

Enables an IPC client to change the current value of a resource, or the values of any resources in an object instance, or the values of any resources of any instances of an object.

Changing individual instances of a multiple-instance resource is supported by setting the ValueID element, which is the index of the resource instance to be changed.

Setting resources in instances of different objects is not supported.

If the resource data is of type string, the ValueData element must be BASE64-encoded.

```xml
<Request>
  <Type>Set</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>              <!-- set a single-instance resource -->
            <ID>0</ID>
            <Value>VALUE</Value>
          </Resource>
          <Resource>              <!-- set a single instance of a multiple-instance resource -->
            <ID>7</ID>
            <SetArrayMode>Replace</SetArrayMode> <!-- Replace will replace the entire array, Update will only affect the specified resource instances -->
            <ResourceInstance>
              <ID>17</ID>
              <Value>VALUE</Value>
            </ResourceInstance>
          </Resource>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>1</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>              <!-- reset a resource back to the default value -->
            <ID>1</ID>
            <Default/>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Set also allows the creation of new object instances through the <Create> tag which is placed inside the <ObjectInstance> tag of a specified object.

If an ID is not specified the client will generate one automatically.

Mandatory resources are automatically created and assigned default values.

Optional resources are not created automatically.

```xml
<Request>
  <Type>Set</Type>
  <SessionID>12345678</SessionID>>
  <Content>
    <Objects>
      <Object>
        <ID>0</ID>
        <ObjectInstance>
          <ID>0</ID>  <!-- optional -->
          <Create/>
          <!-- further resources may be specified for this instance -->
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Set also allows the creation of an optional resource through the <Create> tag which is placed inside the <Resource> tag. The default value is assigned and can be overridden in the same request:

```xml
<Request>
  <Type>Set</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>1</ID>
          <Resource>
            <ID>2</ID>
            <Create/>     <!-- default value assigned -->
          </Resource>
          <Resource>
            <ID>3</ID>
            <Create/>     <!-- specified value assigned -->
            <Value>VALUE</Value>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: the resource or resources were changed within the daemon. The Content element contains the outcome of each resource.

```xml
<Response>
  <Type>Set</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>0</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Created/>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>     <!-- resource modified -->
            <ID>0</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
          <Resource>     <!-- array modified -->
            <ID>7</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
        <ObjectInstance>
          <ID>1</ID>
          <Resource>     <!-- created with default -->
            <ID>2</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
          <Resource>
            <ID>3</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Response>
```

## Subscribe

An IPC client can subscribe to two types of events. In both cases, an asynchronous IPC notification message will be sent when the target resource is affected.

Currently it is possible for an IPC client to subscribe to both types of events on a single resource. This is likely to change because a resource that is both writable and executable may not be supported in future due to issues with the LWM2M standard.

### SubscribeToExecute

This enables an IPC client to request that notifications be sent to it if a LWM2M Execute operation is performed on the target resource. The object ID, object instance ID and resource ID must be specified.

```xml
<Request>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>4</ID>
            <SubscribeToExecute/>
          </Resource>
          <Resource>
            <ID>5</ID>
            <SubscribeToExecute/>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: indicates that the daemon has registered the subscription successfully. Notifications will be sent by the daemon when Execute operations are performed on the target resource.

```xml
<Response>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>4</ID>
            <SubscribeToExecute/>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
          <Resource>
            <ID>5</ID>
            <SubscribeToExecute/>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Response>
```

### SubscribeToChange

Enables an IPC client to request that notifications be sent to it if an operation that changes state (such as LWM2M Write, or an IPC client set) is performed on the target resource, or any resource on the target object instance.

```xml
<Request>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <SubscribeToChange/>
          </Resource>
          <Resource>
            <ID>1</ID>
            <SubscribeToChange/>
          </Resource>
          <Resource>
            <ID>2</ID>
            <SubscribeToChange/>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: indicates that the daemon has registered the subscription successfully. Notifications will be sent by the daemon when operations that change the target resource are performed.

The current value of all resources that are part of the subscription are also returned, in order to avoid a subscribe/get race condition.

```xml
<Response>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
       <ID>3</ID>
       <ObjectInstance>
         <ID>0</ID>
         <Resource>
          <ID>0</ID>
          <SubscribeToChange/>
          <Result>
            <Error>Success</Error>
          </Result>
         </Resource>
         <Resource>
          <ID>1</ID>
          <SubscribeToChange/>
          <Result>
            <Error>AlreadySubscribed</Error>
          </Result>
         </Resource>
         <Resource>
          <ID>2</ID>
          <Subscribed/>
          <SubscribeToChange/>
          <Result>
            <Error>Success</Error>
          </Result>
         </Resource>
         <!-- rest of resources omitted -->
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Response>
```

## Unsubscribe

Unsubscribe enables an IPC client to discontinue a subscription to a resource, and prevents it receiving future notifications on the subscribed events.

```xml
<Request>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <CancelSubscribeToChange/>
          </Resource>
          <Resource>
            <ID>1</ID>
            <CancelSubscribeToExecute/>
          </Resource>
        </Instance>
      </Object>
      <Object>
        <ID>4</ID>
        <CancelSubscribeToChange/>
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: indicates that the subscription has been removed on the target resource.

```xml
<Response>
  <Type>Subscribe</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <CancelSubscribeToChange/>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
          <Resource>
            <ID>1</ID>
            <CancelSubscribeToExecute/>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </Instance>
      </Object>
      <Object>
        <ID>4</ID>
        <CancelSubscribeToChange/>
        <Result>
          <Error>Success</Error>
        </Result>
      </Object>
    </Objects>
  </Content>
</Response>
```

## Delete

Enables an IPC client to request the removal of a hosted resource instance, resource, or all resources of an object instance, or all object instances of an object.

```xml
<Request>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>1</ID>
          </Resource>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>4</ID>
        <ObjectInstance>
          <ID>0</ID>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>1</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>1</ID>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>1</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
      </Object>
      <Object>
        <ID>4</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Result>
            <Error>Success</Error>
          </Result>
          <Resource>
            <ID>0</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
          <!-- ... -->
        </ObjectInstance>
      </Object>
      <Object>
        <ID>1</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>1</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Response>
```

For multiple-instance resources, it is possible to delete a range of indices. Note that this deletes any indices within the range and it is not an error if an index does not exist.

The range is specified by a start and end index. The range is inclusive of the start index, and exclusive of the end index. Therefore a range of (Start=0, EndExclusive=100) will delete valid indexes between 0 and 99 inclusive.

```xml
<Request>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Objects>
      <Object>
        <ID>0</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <ResourceInstance>
              <IDRange>
                <Start>0</Start>
                <EndExclusive>100</EndExclusive>
              </IDRange>
            </ResourceInstance>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Request>
```

Response code 200: indicates that the specified resource, or object instance, has been deleted and is no longer hosted by the daemon.

```xml
<Response>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>0</ID>
        <ObjectInstance>
        <ID>0</ID>
          <Resource>
            <ID>0</ID>
            <Result>
              <Error>Success</Error>
            </Result>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Response>
```

## Notifications

Notifications are sent to all subscribed IPC clients when an event occurs.

### Execute Notification

The Execute Notification is sent by the daemon to all subscribed-to-execute IPC clients when an LWM2M Execute operation is performed on a resource that supports the Execute operation. Any content provided in the LWM2M Delete operation is included in the notification - it is up to the IPC client to decode this data.

Currently the Execute notification returns a single Opaque value however it will be updated to the following to reflect the new method of Execute notifications described in the latest LWM2M Specification.

```xml
<Notification>
  <Type>ServerExecute</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
        <ID>3</ID>
        <ObjectInstance>
          <ID>0</ID>
          <Resource>
            <ID>4</ID>
            <Parameter>
              <Name>Param1</Name>
              <Value>VALUE</Value>
            </Parameter>
          </Resource>
        </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Notification>
```

### Change Notification

The Change Notification is sent by the daemon to all subscribed-to-change IPC clients when an LWM2M Write operation is performed on a resource that supports the Write operation, or a local Set operation occurs. In both cases, the operation must change the resource value for a notification to be generated.

Currently the <ChangeType> node is not supported.

```xml
<Notification>
  <Type>ServerChange</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Objects>
      <Object>
       <ID>3</ID>
       <ChangeType>Modified</ChangeType>
       <ObjectInstance>
         <ID>0</ID>
         <ChangeType>Modified</ChangeType>
         <Resource>
          <ID>0</ID>
          <ChangeType>Modified</ChangeType>
          <Value>VALUE</Value>
         </Resource>
         <Resource>
          <ID>1</ID>
          <ChangeType>Deleted</ChangeType>
         </Resource>
         <Resource>
          <ID>2</ID>
          <ChangeType>Created</ChangeType>
          <Value>VALUE</Value>
         </Resource>
       </ObjectInstance>
       <ObjectInstance>
         <ID>1</ID>
         <ChangeType>Deleted</ChangeType>
         <Resource>
           <!-- ... -->
         </Resource>
         <!-- all deleted resources beneath this instance are also present -->
       </ObjectInstance>
      </Object>
    </Objects>
  </Content>
</Notification>
```

# LWM2M Server Daemon

## ListClients

Used to retrieve the list of known LWM2M clients and list of currently registered objects and object instances for each client.

```xml
<Request>
  <Type>ListClients</Type>
  <SessionID>12345678</SessionID>
</Request>
```

```xml
<Response>
  <Type>ListClients</Type>
  <SessionID>12345678</SessionID>   <!-- optional -->
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Status>Registered</Status>
        <Objects>
          <Object>
            <ID>2</ID>   <!-- object type defined, no instances -->
          </Object>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
            </ObjectInstance>
          </Object>
          <Object>
            <ID>4</ID>
            <ObjectInstance>
              <ID>0</ID>
            </ObjectInstance>
            <ObjectInstance>
              <ID>1</ID>
            </ObjectInstance>
          </Object>
          <!-- ... -->
        </Objects>
      </Client>
      <Client>
        <ID>TestClient2</ID>
        <Status>Registered</Status>
        <Objects>
          <!-- ... -->
        </Objects>
      </Client>
      <!-- can also convey status of previously registered clients (no entities?) -->
      <Client>
        <ID>SleepyClient1</ID>
        <Status>Deregistered</Status>
      </Client>
      <!-- ... -->
    </Clients>
  </Content>
</Response>
```

## Read

```xml
<Request>
  <Type>Read</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>0</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Read</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>0</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <Value>VALUE</Value>
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## Discover

Discover enables a server to discover attributes of an individual resource, all the resources of an object instance, or all the object instances of an object.

```xml
<Request>
  <Type>Discover</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

TODO: add more examples

```xml
<Response>
  <Type>Discover</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <!-- TODO attributes -->
            <ObjectInstance>
              <ID>0</ID>
              <!-- TODO attributes -->
              <Resource>
                <ID>0</ID>
                <!-- TODO attributes -->
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## Write

```xml
<Request>
  <Type>Write</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <DefaultWriteMode>Update</DefaultWriteMode>
        <Objects>
          <Object>
            <ID>0</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <Value>VALUE</Value>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Write</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <DefaultWriteMode>Update</DefaultWriteMode>
        <Objects>
          <Object>
            <ID>0</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## WriteAttributes

```xml
<Request>
  <Type>WriteAttributes</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>13</ID>
                <Attribute>
                  <Link>gt</Link>
                  <Value>10</Value>
                  <ValueType>Integer</ValueType>
                </Attribute>
                <Attribute>
                  <Link>lt</Link>
                  <Value>-2.5</Value>
                  <ValueType>Float</ValueType>
                </Attribute>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>WriteAttributes</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>13</ID>
                <Result>
                  <Error>FlowM2MError_Success</Error>
                </Result>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## Execute

```xml
<Request>
  <Type>Execute</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>4</ID>
                <Value>VALUE</Value>  <!-- optional opaque data -->
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Execute</Type>
  <SessionID>12345678</SessionID>
  <Code>204</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>4</ID>
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## Delete

```xml
<Request>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>1</ID>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Delete</Type>
  <SessionID>12345678</SessionID>
  <Code>202</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>1</ID>
              <Result>
                <Error>Success</Error>
              </Result>
              <Resource>
                <ID>0</ID>
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
              <Resource>
                <ID>1</ID>
                <Result>
                  <Error>Success</Error>
                </Result>
              </Resource>
              <!-- ... -->
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## Observe

```xml
<Request>
  <Type>Observe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <Observe/>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Observe</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <Observe/>
                <Result>
                  <Error>Success</Error>
                </Result
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>
```

## CancelObserve

```xml
<Request>
  <Type>Observe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <CancelObserve/>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Request>
```

```xml
<Response>
  <Type>Observe</Type>
  <SessionID>12345678</SessionID>
  <Code>200</Code>
  <Content>
    <Clients>
      <Client>
        <ID>TestClient1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>0</ID>
                <CancelObserve/>
                <Result>
                  <Error>Success</Error>
                </Result
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Response>

## Notifications

### Observe Notification

```xml
<Notification>
  <Type>Observe</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
        <Objects>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
              <Resource>
                <ID>1</ID>
                <Observe></Observe>
                <Value>QXdhIENsaWVudA==</Value>
              </Resource>
            </ObjectInstance>
          </Object>
        </Objects>
      </Client>
    </Clients>
  </Content>
</Notification>
```

### Register Event

Through the server API, it will be possible to receive client registration events.

When a new client connects to a server, receive a client register event with the client's endpoint name.

The server may combine multiple clients into a single event.

```xml
<Notification>
  <Type>ClientRegister</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
        <Objects>
          <Object>
            <ID>2</ID>   <!-- object type defined, no instances -->
          </Object>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
            </ObjectInstance>
          </Object>
          <!-- ... -->
        </Objects>
      </Client>
    </Clients>
  </Content>
</Notification>
```

### Deregister Event

Through the server API, it will be possible to receive client deregistration events.

When a client disconnects from a server, receive a deregister notification with the client's endpoint name.

The server may combine multiple clients into a single event.

```xml
<Notification>
  <Type>ClientDeregister</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
      </Client>
    </Clients>
  </Content>
</Notification>
```

### Update Event

Through the server API, it will be possible to receive client update events.

While a client is registered to a server, it will send periodic updates. The API receives an update event with the client's endpoint name.

The server may combine multiple clients into a single event.

```xml
<Notification>
  <Type>ClientUpdate</Type>
  <SessionID>12345678</SessionID>
  <Content>
    <Clients>
      <Client>
        <ID>TestIMG1</ID>
        <Objects>
          <Object>
            <ID>2</ID>
            <ObjectInstance>      <!-- new instance -->
              <ID>4</ID>
            </ObjectInstance>
          </Object>
          <Object>
            <ID>3</ID>
            <ObjectInstance>
              <ID>0</ID>
            </ObjectInstance>
          </Object>
          <!-- ... -->
        </Objects>
      </Client>
    </Clients>
  </Content>
</Notification>
```

