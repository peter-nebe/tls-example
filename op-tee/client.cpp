#include "client-ta.h"
#include "tee_client_api.h"
#include <stdexcept>
#include <iostream>
using namespace std;

class TeeClient
{
  TEEC_Context context;

public:
  TeeClient()
  {
    TEEC_Result res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS)
      throw runtime_error("TEEC_InitializeContext");
  }

  ~TeeClient()
  {
    TEEC_FinalizeContext(&context);
  }

  bool invokeCommand(uint32_t commandId)
  {
    TEEC_Session session;
    const TEEC_UUID uuid = CLIENT_TA_UUID;
    uint32_t errOrigin;
    TEEC_Result res = TEEC_OpenSession(&context, &session, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &errOrigin);
    if (res != TEEC_SUCCESS)
    {
      cerr << "TEEC_OpenSession failed with code 0x" << hex << res << " origin 0x" << errOrigin << endl;
      return false;
    }

    TEEC_Operation op{};
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    res = TEEC_InvokeCommand(&session, commandId, &op, &errOrigin);
    if (res != TEEC_SUCCESS)
      cerr << "TEEC_InvokeCommand failed with code 0x" << hex << res << " origin 0x" << errOrigin << endl;

    TEEC_CloseSession(&session);

    return res == TEEC_SUCCESS;
  }
};

int main()
{
  TeeClient teeClient;
  bool success = teeClient.invokeCommand(CLIENT_TA_CMD_SEND);
  cout << "success: " << boolalpha << success << endl;

  return 0;
}
