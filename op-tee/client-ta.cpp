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
#include <tee_internal_api.h>
#include <trace.h>
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

TEE_Result TA_InvokeCommandEntryPoint(void *sessionContext, uint32_t commandID,
                                      uint32_t paramTypes, TEE_Param params[TEE_NUM_PARAMS])
{
	if(!sessionContext || commandID != CLIENT_TA_CMD_REQUEST)
    return TEE_ERROR_BAD_PARAMETERS;

  uint32_t paramTypesExp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT, TEE_PARAM_TYPE_NONE,
                                           TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
  if (paramTypes != paramTypesExp)
  {
    EMSG("got paramTypes 0x%x, expected 0x%x", paramTypes, paramTypesExp);
    return TEE_ERROR_BAD_PARAMETERS;
  }

  ClientSession *session = static_cast<ClientSession*>(sessionContext);
  int err = session->request(static_cast<char*>(params[0].memref.buffer), &params[0].memref.size);
  if(err)
  {
    EMSG("session-request %d", err);
    return TEE_ERROR_GENERIC;
  }

  return TEE_SUCCESS;
}
