### TLS client [TA](## "Trusted Application")
This is an example TLS client running in a [TEE](## "Trusted Execution Environment") implemented by [OP-TEE](## "Open Portable TEE").

#### How to build
To run the app on a device you need a suitably built OP-TEE OS. I only have a Raspberry Pi 4, but [OP-TEE](https://github.com/OP-TEE) doesn't officially support it yet. A few adjustments are also necessary to be able to use the TLS protocol. To build OP-TEE for the RPi4, you can follow the [building instructions](https://github.com/peter-nebe/optee_os/tree/tls-client-ta#building-instructions) in my branch. Be sure to use the *tls-client-ta* branch.

When OP-TEE is running on your device, you can build and install the app similar to the [mk](mk) and [inst](inst) scripts.

#### Testing
It was tested on a Raspberry Pi 4 using the server from the [main folder](https://github.com/peter-nebe/tls-example) running on a PC. Here is the test run:
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
