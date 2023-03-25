/*
 *  TCP/IP or UDP/IP networking functions
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "tee_isocket.h"
#include "tee_tcpsocket.h"
#include "trace.h"
#include <stdlib.h>
#include <string.h>

/*
 * Initialize a context
 */
void mbedtls_net_init( mbedtls_net_context *ctx )
{
  ctx->fd = -1;
}

struct sock_handle
{
  TEE_iSocketHandle ctx;
  TEE_iSocket *socket;
};

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host,
                         const char *port, int proto )
{
  int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
  struct sock_handle h = { };
  TEE_tcpSocket_Setup setup = { };
  uint32_t protocolError = 0;

  if(proto != MBEDTLS_NET_PROTO_TCP)
  {
    EMSG("Currently only the TCP transport protocol is supported.");
    return MBEDTLS_ERR_NET_BAD_INPUT_DATA;
  }

  setup.ipVersion = TEE_IP_VERSION_DC;
  setup.server_port = strtoul(port, 0, 10);
  setup.server_addr = strdup(host);
  if (!setup.server_addr)
    return TEE_ERROR_OUT_OF_MEMORY;

  h.socket = TEE_tcpSocket;
  TEE_Result res = h.socket->open(&h.ctx, &setup, &protocolError);
  free(setup.server_addr);
  if (res == TEE_SUCCESS)
  {
    // This is a cheap solution to not having to change the context structure.
    // 32 address bits are sufficient for the intended target platform.
    ctx->fd = (int)(uint64_t)h.ctx;
    ret = 0;
  }
  else
    ret = MBEDTLS_ERR_NET_CONNECT_FAILED;

  FMSG("sock-open %x prot-err %x", res, protocolError);
  return( ret );
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
  int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
  struct sock_handle h =
  {
    .ctx = (TEE_iSocketHandle)(uint64_t)((mbedtls_net_context*)ctx)->fd,
    .socket = TEE_tcpSocket
  };
  uint32_t nbytes = len;
  TEE_Result res = h.socket->recv(h.ctx, buf, &nbytes, TEE_TIMEOUT_INFINITE);
  ret = (res == TEE_SUCCESS) ? (int)nbytes : MBEDTLS_ERR_NET_RECV_FAILED;

  FMSG("sock-recv %x nbytes %u", res, nbytes);
  return( ret );
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf,
                              size_t len, uint32_t timeout )
{
  int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
  struct sock_handle h =
  {
    .ctx = (TEE_iSocketHandle)(uint64_t)((mbedtls_net_context*)ctx)->fd,
    .socket = TEE_tcpSocket
  };
  uint32_t nbytes = len;
  TEE_Result res = h.socket->recv(h.ctx, buf, &nbytes, timeout);
  ret = (res == TEE_SUCCESS) ? (int)nbytes : MBEDTLS_ERR_NET_RECV_FAILED;

  FMSG("sock-recv %x nbytes %u", res, nbytes);
  return( ret );
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
  int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
  struct sock_handle h =
  {
    .ctx = (TEE_iSocketHandle)(uint64_t)((mbedtls_net_context*)ctx)->fd,
    .socket = TEE_tcpSocket
  };
  uint32_t nbytes = len;
  TEE_Result res = h.socket->send(h.ctx, buf, &nbytes, 0);
  ret = (res == TEE_SUCCESS) ? (int)nbytes : MBEDTLS_ERR_NET_SEND_FAILED;

  FMSG("sock-send %x nbytes %u", res, nbytes);
  return( ret );
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
  if( ctx->fd == -1 )
    return;

  struct sock_handle h =
  {
    .ctx = (TEE_iSocketHandle)(uint64_t)((mbedtls_net_context*)ctx)->fd,
    .socket = TEE_tcpSocket
  };
  h.socket->close(h.ctx);

  ctx->fd = -1;
}
