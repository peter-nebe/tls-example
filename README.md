# tls-example
Example of an encrypted connection via TLS with mutual authentication using X.509 certificates

### Dependencies
- [Boost](https://www.boost.org)
- [OpenSSL](https://www.openssl.org)
- [OP-TEE](https://github.com/OP-TEE) (only for the TLS client TA)

### Overview
The main folder contains an example TLS server and client suitable for all platforms. The server simply sends back all received words in reverse order.

In [op-tee](op-tee) there is a TLS client as a trusted application (TA). It was tested on a **Raspberry Pi 4**.
