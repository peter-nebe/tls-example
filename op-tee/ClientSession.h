/*
 * tls-example
 * Copyright (c) 2023 Peter Nebe <mail@peter-nebe.dev>
 *
 * This file is part of tls-example.
 *
 * tls-example is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tls-example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tls-example.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CLIENTSESSION_H_
#define CLIENTSESSION_H_

#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"

class ClientSession
{
public:
  ClientSession();
  ~ClientSession();
  int open();
  void close();
  int request(char *buf, size_t *len);

private:
  mbedtls_net_context server_fd;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_x509_crt cacert;

  enum class State
  {
    undefined,
    initialized,
    opening,
    connected,
    open
  }
  state;
};

#endif /* CLIENTSESSION_H_ */
