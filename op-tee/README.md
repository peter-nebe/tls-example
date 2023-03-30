### TLS client [TA](## "Trusted Application")
This is an example TLS client running in a [TEE](## "Trusted Execution Environment") implemented by [OP-TEE](## "Open Portable TEE").

All information coming out of the [REE](## "Rich Execution Environment") is insecure: It can be spied on by unauthorized persons or forged by attackers. The goal is therefore to keep as much of the required information as possible within the TEE. In this example, the following data is fixed in the TA:
- the [server name](ClientSession.cpp#L24)
- the server port number
- the hostname to check against the received server certificate
- the [CA root certificate](tls-example-ca.c)

However, some information must be exchanged with the REE, such as in this example the request sent to the server and the response received. It is important to minimize the risks associated with insecure information. For example, the TA could reject a request that it recognized as a fake. However, recognizing a fake is not trivial and rejecting it outright might make it easier for skilled attackers to trick the detection.

Therefore, a different strategy is implemented in this example. This TA does not reject any request, but determines a special **confidence measure** for each request. This confidence measure is encoded and prepended to the request before it is sent to the server. In this way, the client TA can communicate any additional security-related information to the server. It is then up to the server how to process the request. It can develop its own strategy to do so.

This example is only intended to show the basic principle. If you look at the code, you will see that [determining the confidence measure](client-ta.cpp#L87) is trivial. The server used is also just a dummy that just sends back the words of the request in reverse order, for example:
```
request: #2 Here's some fake news.
I/TA: raw response: news. fake some Here's #2 §46!010+
```

#### How to build
To run the app on a device you need a suitably built OP-TEE OS. I only have a Raspberry Pi 4, but [OP-TEE](https://github.com/OP-TEE) doesn't officially support it yet. A few adjustments are also necessary to be able to use the TLS protocol. To build OP-TEE for the RPi4, you can follow the [building instructions](https://github.com/peter-nebe/optee_os/tree/tls-client-ta#building-instructions) in my branch. Be sure to use the *tls-client-ta* branch.

When OP-TEE is running on your device, you can build and install the app similar to the [mk](mk) and [inst](inst) scripts.

#### Testing
It was tested on a Raspberry Pi 4 using the server from the [main folder](https://github.com/peter-nebe/tls-example) running on a PC. The server must have been compiled with the *-DNO_CLIENT_AUTHENTICATION* option. Here is the test run:
```
# ./tls-example-client

  . Seeding the random number generator... ok
  . Loading the CA root certificate ... ok (0 skipped)
  . Connecting to tcp/192.167.1.1/4433... ok
  . Setting up the SSL/TLS structure... ok
  . Performing the SSL/TLS handshake... ok
  . Verifying peer X.509 certificate... ok
  > Write to server: 31 bytes written

Hello from TA how are you dude?
  < Read from server: 31 bytes read

dude? you are how TA from Hello
success: true
# 
```
