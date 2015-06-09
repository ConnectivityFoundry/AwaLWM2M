
![](doc/img.png)

----

# Awa LightweightM2M. 

## Connecting the LWM2M Client to 3rd party servers.

### wakaama (bootstrap)  / leshan (server).

 awa_clientd can connect to both the wakaama and leshan servers. These are third-party LWM2M servers provided by the Eclipse Foundation. The follow instructions outline how to use the wakaama bootstrap server and a leshan server.

 Compile and run the wakaama bootstrap server. You may need to edit bootstrap_server.ini to decrease the "lifetime" setting, though the other settings are pre-configured for leshan:

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

 Run the Imagination LWM2M client with this bootstrap server address specified:

    $ build/core/src/client/awa_clientd --bootstrap coap://0.0.0.0:15678 --verbose --endPointName imaginationtest

 Open http://leshan.eclipse.org/ in your web browser.

### wakaama (server)

 Assuming you have already built the bootstrap server, modify the configuration and start the bootstrap server:

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

 Compile and run the wakaama server:

    $ mkdir server
    $ cd server
    $ server> cmake ../wakaama/tests/server
    $ server> make
    $ server> ./lwm2mserver

 Run the Imagination LWM2M client:

    $ build/core/src/client/awa_clientd --bootstrap coap://0.0.0.0:15678 --verbose --endPointName imaginationtest

 The wakaama client will show the registration attempt.

