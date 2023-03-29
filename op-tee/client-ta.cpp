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

#include "client-ta.h"
#include "ClientSession.h"
extern "C" {
#include "tee_internal_api.h"
#include <string.h>
}

TEE_Result TA_CreateEntryPoint()
{
  return TEE_SUCCESS;
}

void TA_DestroyEntryPoint()
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t, TEE_Param[], void **sessionContext)
{
  ClientSession *session = new ClientSession();
  if(!session)
    return TEE_ERROR_OUT_OF_MEMORY;

  int err = session->open();
  if(err)
  {
    EMSG("session-open %d", err);
    delete session;
    return TEE_ERROR_GENERIC;
  }

  *sessionContext = session;
  return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sessionContext)
{
  delete static_cast<ClientSession*>(sessionContext);
}

/*
 * Substitute for strstr since buf does not have to be null-terminated
 */
static bool contains(const char *buf, size_t size, const char *substr)
{
  size_t subsiz = strlen(substr);
  if(subsiz > size)
    return false;

  auto matches = [](const char *p1, const char *p2, size_t size)
                 {
                   while(size--)
                     if(*p1++ != *p2++)
                       return false;
                   return true;
                 };
  size_t n = size - subsiz + 1;
  for(size_t i = 0; i < n; i++)
    if(matches(buf + i, substr, subsiz))
      return true;

  return false;
}

/*
 * See the README for an explanation of this function
 */
static Buffer addConfidenceMeasure(const decltype(TEE_Param::memref) &memref)
{
  const char *confMeasure[]
  {
    "§46!009+ ",
    "§46!010+ "
  };
  int ind = contains(static_cast<char*>(memref.buffer), memref.size, "fake") ? 1 : 0;
  size_t addSize = strlen(confMeasure[ind]);

  Buffer buf(addSize + memref.size);
  strcpy(reinterpret_cast<char*>(buf.ptr), confMeasure[ind]);
  memcpy(buf.ptr + addSize, memref.buffer, memref.size);
  buf.contsize = buf.bufsize;

  return buf;
}

TEE_Result TA_InvokeCommandEntryPoint(void *sessionContext, uint32_t commandID,
                                      uint32_t paramTypes, TEE_Param params[TEE_NUM_PARAMS])
{
  if(!sessionContext || commandID != CLIENT_TA_CMD_REQUEST)
    return TEE_ERROR_BAD_PARAMETERS;

  uint32_t paramTypesExp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                           TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
  if (paramTypes != paramTypesExp)
  {
    EMSG("got paramTypes 0x%x, expected 0x%x", paramTypes, paramTypesExp);
    return TEE_ERROR_BAD_PARAMETERS;
  }

  Buffer requ = addConfidenceMeasure(params[0].memref);
  size_t addSize = requ.contsize - params[0].memref.size;
  Buffer resp(params[1].memref.size + addSize);

  int err = static_cast<ClientSession*>(sessionContext)->doRequest(requ, resp);
  if(err)
  {
    EMSG("session-request %d", err);
    return TEE_ERROR_GENERIC;
  }

  IMSG("raw response: %.*s", resp.contsize, resp.ptr);

  size_t respSize = resp.contsize;
  if(respSize >= addSize)
    respSize -= addSize;

  memcpy(params[1].memref.buffer, resp.ptr, respSize);
  if(params[1].memref.size > respSize)
    params[1].memref.size = respSize;

  return TEE_SUCCESS;
}
