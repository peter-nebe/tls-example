# tls-example
Example of an encrypted connection via TLS with mutual authentication using X.509 certificates

### Dependencies
- [Boost](https://www.boost.org)
- [OpenSSL](https://www.openssl.org)
- [OP-TEE](https://github.com/OP-TEE) (only for the TLS client TA)

### Details
The main folder contains an example TLS server and client suitable for all platforms. The server simply sends back all received words in reverse order.

In [op-tee](op-tee) there is a TLS client as a trusted application (TA). It runs in a [TEE](## "Trusted Execution Environment") implemented by [OP-TEE](## "Open Portable TEE").

Here is the run on a Raspberry Pi 4:

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
