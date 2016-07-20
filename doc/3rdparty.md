
![](images/img.png)

----

# Awa LightweightM2M. 

## Connecting the Awa LightweightM2M client to third-party servers.

### Wakaama (bootstrap) / Leshan (server).

The *awa_clientd*  daemon can connect to both the Wakaama and Leshan servers. These are third-party LWM2M servers provided by the Eclipse Foundation. The following instructions outline how to use the Wakaama bootstrap server and a Leshan server.

Compile and run the Wakaama bootstrap server. You may need to edit *bootstrap_server.ini* to decrease the *lifetime* setting, though the remaining settings are pre-configured for Leshan:
````
$ git clone https://github.com/eclipse/wakaama.git
$ mkdir bootstrap
$ cd bootstrap
$ bootstrap> cmake ../wakaama/tests/bootstrap_server
$ bootstrap> make
$ bootstrap> cp ../wakaama/tests/bootstrap_server/bootstrap_server.ini .

$ bootstrap> vi bootstrap_server.ini

# Information for the Leshan sandbox server hosted by
# the Eclipse Foundation
[Server]
id=2
uri=coap://leshan.eclipse.org:5683
bootstrap=no
lifetime=300   << change this to 20
security=NoSec

$ bootstrap> ./bootstrap_server -p 15678
````
Run the Imagination LWM2M client with the above bootstrap server address specified:
````
$ build/core/src/client/awa_clientd --bootstrap coap://0.0.0.0:15678 --verbose --endPointName imaginationtest
````
Open http://leshan.eclipse.org/ in your web browser.

### Wakaama (server).

Assuming you have already built the bootstrap server, modify the configuration and start the bootstrap server:
````
$ bootstrap> vi bootstrap_server.ini

# Information for the Leshan sandbox server hosted by
# the Eclipse Foundation
[Server]
id=2
uri=coap://0.0.0.0:5683  << change this to local host
bootstrap=no
lifetime=10
security=NoSec

$ bootstrap> ./bootstrap_server -p 15678
````
Compile and run the wakaama server:
````
$ mkdir server
$ cd server
$ server> cmake ../wakaama/tests/server
$ server> make
$ server> ./lwm2mserver
````
Run the Awa LightweightM2M client:
````
$ build/core/src/client/awa_clientd --bootstrap coap://0.0.0.0:15678 --verbose --endPointName imaginationtest
````
The Wakaama client will show the registration attempt.

----
----
