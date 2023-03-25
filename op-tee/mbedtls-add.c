#include "mbedtls/entropy_poll.h"
#include "tee_internal_api.h"

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
  (void)data;

  TEE_GenerateRandom(output, len);
  *olen = len;

  return 0;
}
