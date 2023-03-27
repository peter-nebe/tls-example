# TLS client [TA](## "Trusted Application")
This is an example TLS client running in a [TEE](## "Trusted Execution Environment") implemented by [OP-TEE](## "Open Portable TEE").

It was tested on a Raspberry Pi 4 using the server from the [main folder](..) running on a PC. Here is the test run:
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
