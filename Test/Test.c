#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Types.h>

// defined in /Test/MockApi.c
ReaiResponse* reai_mock_request (
    Reai*         reai,
    ReaiRequest*  request,
    ReaiResponse* response,
    CString       endpoint_str,
    Uint32*       http_code
);

// What things do I want tested?
// Objects are created and properties are set correctly
// Responses are parsed correctly : Request -> MockAPI -> Parsed response -> Check
// I don't want to test the working of the MockAPI

int main() {
    Reai* reai = reai_create ("https://mock.reveng.api", "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX");
    reai_set_mock_handler (reai, reai_mock_request);

    ReaiResponse response = {0};
    reai_response_init (&response);

    if (!reai_upload_file (reai, &response, "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")) {
        REAI_LOG_ERROR ("auth check failure");
        return 1;
    }

    return 0;
}
