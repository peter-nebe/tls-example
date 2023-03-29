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

#include "ClientSession.h"
#include "mbedtls/error.h"

static const char *serverName = "192.167.1.1";
static const char *serverPort = "44077";
static const char *certHostName = "tls-example-server";

ClientSession::ClientSession()
{
  mbedtls_net_init(&server_fd);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&cacert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init(&entropy);

  state = State::initialized;
}

ClientSession::~ClientSession()
{
  close();

  mbedtls_net_free(&server_fd);
  mbedtls_x509_crt_free(&cacert);
  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
}

int ClientSession::open()
{
  state = State::opening;

  // seed the random number generator
  const unsigned char custom[] = "custom1";
  int err = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, custom, sizeof custom);
  if(err)
    return err;

  // load the CA root certificate
  extern const unsigned char tls_example_ca[];
  extern const size_t tls_example_ca_len;
  err = mbedtls_x509_crt_parse(&cacert, tls_example_ca, tls_example_ca_len);
  if(err < 0)
    return err;

  // setup the SSL/TLS structure
  err = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
  if(err)
    return err;

  mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
  err = mbedtls_ssl_setup(&ssl, &conf);
  if(err)
    return err;

  err = mbedtls_ssl_set_hostname(&ssl, certHostName);
  if(err)
    return err;

  // connect to server
  err = mbedtls_net_connect(&server_fd, serverName, serverPort, MBEDTLS_NET_PROTO_TCP);
  if(err)
    return err;

  mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

  state = State::connected;

  // perform the SSL/TLS handshake
  while((err = mbedtls_ssl_handshake(&ssl)) != 0)
  {
    if(err != MBEDTLS_ERR_SSL_WANT_READ && err != MBEDTLS_ERR_SSL_WANT_WRITE)
      return err;
  }

  // verify the server certificate
  err = mbedtls_ssl_get_verify_result(&ssl);
  if(err)
    return err;

  state = State::open;

  return 0;
}

void ClientSession::close()
{
  if(state >= State::connected)
    mbedtls_ssl_close_notify(&ssl);

  state = State::initialized;
}

int ClientSession::request(char *buf, size_t *len)
{
  if(state != State::open)
    return MBEDTLS_ERR_ERROR_GENERIC_ERROR;

  // send to server
  int ret;
  while((ret = mbedtls_ssl_write(&ssl, reinterpret_cast<unsigned char*>(buf), *len)) <= 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      return ret;
  }

  // Receive from server.
  // We expect our example server to return the same number of characters as it received.
  const size_t toReceive = ret < *len ? ret : *len;
  size_t received = 0;

  do
  {
    ret = mbedtls_ssl_read(&ssl, reinterpret_cast<unsigned char*>(buf) + received, toReceive - received);
    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;

    if(ret <= 0)
      break;

    received += ret;
  }
  while(received < toReceive);

  *len = received;
  if(ret >= 0)
    ret = received < toReceive ? MBEDTLS_ERR_SSL_CONN_EOF : 0;

  return ret;
}
