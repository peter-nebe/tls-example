#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "tee_isocket.h"
#include "tee_tcpsocket.h"
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

      ret = 0;
    }

    DMSG("sock-open %x prot-err %x", res, protocolError);
    return( ret );
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;

    return( ret );
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf,
                              size_t len, uint32_t timeout )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;

    /* This call will not block */
    return( mbedtls_net_recv( ctx, buf, len ) );
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;

    return( ret );
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if( ctx->fd == -1 )
        return;

    ctx->fd = -1;
}
