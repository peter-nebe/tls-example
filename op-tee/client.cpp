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
#include "tee_client_api.h"
#include <stdexcept>
#include <iostream>
#include <vector>
using namespace std;

class TeeClient
{
  TEEC_Context context;
  TEEC_Session session;
  bool sessionOpen = false;

public:
  TeeClient()
  {
    TEEC_Result res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS)
      throw runtime_error("TEEC_InitializeContext");
  }

  ~TeeClient()
  {
    if(sessionOpen)
      TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
  }

  bool openSession()
  {
    const TEEC_UUID uuid = CLIENT_TA_UUID;
    uint32_t errOrigin;
    TEEC_Result res = TEEC_OpenSession(&context, &session, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &errOrigin);
    if (res != TEEC_SUCCESS)
    {
      cerr << "TEEC_OpenSession failed with code 0x" << hex << res << " origin 0x" << errOrigin << endl;
      return false;
    }

    sessionOpen = true;
    return true;
  }

  void closeSession()
  {
    if(sessionOpen)
    {
      TEEC_CloseSession(&session);
      sessionOpen = false;
    }
  }

  bool doRequest(const string &request, string &response)
  {
    if(!sessionOpen)
      return false;

    TEEC_Operation op{};
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);

    // because of the input type, the buffer is not modified
    op.params[0].tmpref.buffer = const_cast<char*>(request.data());
    op.params[0].tmpref.size = request.size();

    // we expect our example server to return the same number of characters as it received
    response.resize(request.size());
    op.params[1].tmpref.buffer = response.data();
    op.params[1].tmpref.size = response.size();

    uint32_t errOrigin;
    TEEC_Result res = TEEC_InvokeCommand(&session, CLIENT_TA_CMD_REQUEST, &op, &errOrigin);
    if (res != TEEC_SUCCESS)
    {
      cerr << "TEEC_InvokeCommand failed with code 0x" << hex << res << " origin 0x" << errOrigin << endl;
      return false;
    }

    if(response.size() > op.params[1].tmpref.size)
      response.resize(op.params[1].tmpref.size);

    return true;
  }
}; // class TeeClient

int main()
{
  cout << "open session... " << flush;

  TeeClient teeClient;
  if(!teeClient.openSession())
    return 1;

  cout << "ok" << endl;

  // a few example requests
  const vector<string> requests
  {
    "#1 Hello via TA!",
    "#2 Here's some fake news.",
    "#3 What's next?"
  };

  for(const string &request : requests)
  {
    cout << "request: " << request << endl;

    string response;
    bool success = teeClient.doRequest(request, response);
    cout << "success: " << boolalpha << success << endl;

    if(success)
      cout << "response: " << response << endl;
    cout << endl;
  }

  return 0;
}
