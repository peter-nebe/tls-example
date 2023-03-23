#include "mbedtls/entropy_poll.h"
#include "tee_internal_api.h"
#include <string.h>

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
  (void)data;

  // HWRNG_SERVICE_UUID
  const TEE_UUID uuid = { 0x6272636D, 0x2019, 0x0201, { 0x42, 0x43, 0x4D, 0x5F, 0x52, 0x4E, 0x47, 0x30 }};
  const uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
  TEE_Param params[TEE_NUM_PARAMS];
  TEE_TASessionHandle session;
  uint32_t origin;
  size_t bytes_read = 0;
  size_t words_read = 0;

  TEE_Result res = TEE_OpenTASession(&uuid, 0, param_types, params, &session, &origin);
  if(res == TEE_SUCCESS)
  {
    const size_t words_to_read = (len + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    while(words_read < words_to_read)
    {
      res = TEE_InvokeTACommand(session, 0, 0 /*PTA_BCM_HWRNG_CMD_GET*/, param_types, params, &origin);
      if(res == TEE_SUCCESS)
      {
        ++words_read;
        size_t nbytes = sizeof(uint32_t);
        if(bytes_read + nbytes > len)
          nbytes = len - bytes_read;
        memcpy(output + bytes_read, &params[0].value.a, nbytes);
        bytes_read += nbytes;
      }
      else
        break;
    }
    TEE_CloseTASession(session);
  }

  DMSG("res %x, %lu bytes read (%lu words)", res, bytes_read, words_read);

  *olen = bytes_read;
  return res == TEE_SUCCESS ? 0 : -1;
}
