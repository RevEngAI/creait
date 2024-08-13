# C RevEngAI Toolkit

`creait` is a library for writing C applications that interact with the RevEngAI API.
creait is currently under development.

## Installation

``` sh
# Clone this repo and cd into it
git clone git@github.com:RevEngAI/creait.git && cd creait

# Configre the build using ninja. Remove -G Ninja if you prefer usign GNU Makefiles (make required)
cmake -B Build -G Ninja

# Build and install creait.
ninja -C Build && sudo ninja -C Build install
```

You can just copy paste this directly in your terminal and it will do everyting for you,
given the following dependencies are installed already.

## Dependencies

Before building, user/developer must have libcurl (development package), git, cmake, make, ninja and pkg-config installed on host system. The package names differ from OS to OS.

## Usage

All struct type names start with `Reai` and all function name starts with `reai_`.
The naming convention for function is in `snake_case` and is in the following format :
`reai_<object-name>_<command>(...)`. Examples are `reai_analysis_info_vec_create()` or
`reai_request()`, where `Reai` is the object name.

There are three main objects you need to interact with. 
- First one is opaque `Reai` object. This is the main object that connects you to RevEng.AI servers.
- Second is `ReaiRequest` which you must use to build your request.
- Last is `ReaiResponse` where you get the response from RevEng.AI servers.

### Configurations 

To connect with RevEng.AI servers, you first need to load a config file. The config file must
usually be present in your home directory and must have name `~/.reait.toml`. This means, it's
a config file for RevEng.AI Toolkit (reait). A very basic config is 

``` toml
apikey = "l1br3"
host = "https://api.reveng.ai/v1"
model = "binnet-0.3-x86"
db_dir_path = "~/.reai"
log_dir_path = "/tmp"
```

To load the config, you must create a `ReaiConfig` object that parses this toml file and stores
the data in it for you to use easily.

``` c
#include <Reai/Config.h>

int main() {
    ReaiConfig *cfg = reai_config_load (Null);
    RETURN_VALUE_IF(!cfg, EXIT_FAILURE, "Failed to load configuration.");
    
    return EXIT_SUCCESS;
}
```

Note that `Null` is passed to `reai_config_load`. This means the config will be loaded from it's
default expected path. Alternatively, you can load the config from your given path as well.

### Making Contact

Next step is to connect with RevEng.AI servers. This is done using a single function call

``` c
#include <Reai/Api/Api.h>

int main() {
    // load config
    
    // These values can be passed without loading config as well if
    // it is desired to be hardcoded ihe the program itself.
    Reai *reai = reai_create (cfg->host, cfg->apikey, cfg->model);
    RETURN_VALUE_IF(!reai, EXIT_FAILURE, "Failed to connect to RevEng.AI servers.");
    
    return EXIT_SUCCESS;
}
```

If you already have host and apikey present in your codebase then you don't need to load
the configuration as well.

### Getting Responses

Before you make any request, you must create a response structure where you'll get
responses from the server. This is done by initializing one.

``` c
#include <Reai/Api/Api.h>

int main() {
    // other code
    
    ReaiResponse response;
    reai_response_init (&response);

}
```

### Making Requests

There are different types of requests and you can go through them in RevEng.AI API docs,
or you can go through them in `Include/Reai/Api/Request.h` header. Name of each request type
is very closely related to the endpoint it'll communicate with, and to create a request,
you need to fill valid data in correspondigly name structure inside `ReaiRequest` object.

For example, if you want to search a binary you'll use the request type `REAI_REQUEST_TYPE_SEARCH`
and fill data in `ReaiRequest::search`.

``` c
#include <Reai/Api/Api.h>

int main() {
    // other code
    
    ReaiRequest request = {0};

    // Build request
    request.type                   = REAI_REQUEST_TYPE_SEARCH;
    request.search.collection_name = "trojan";
    reai_request (reai, &request, &response);

    // Verify that you got correct response
    RETURN_VALUE_IF (
        response.type != REAI_RESPONSE_TYPE_SEARCH,
        EXIT_FAILURE,
        "Failed to perform search : %s.\n",
        response.raw.data
    );

    // Go over all query results and print values
    if (response.search.success) {
        REAI_VEC_FOREACH (response.search.query_results, result, {
            PRINT_ERR ("%s\n", result->binary_name);
        });
    } else {
        PRINT_ERR ("Search failed.\n");
    }
}
```

`PRINT_ERR` is just a macro to help debugging. It prepends the name of the function
which issued the message and hence helps finding errors quickly.

### Extra Notes

Respones are reset everytime a new request is made. This means anything inside the
response you want to keep, you must clone. For vectors there are `clone_create` methods already
implemented, and whatever you clone is your responsibility and you must destroy those
explicitly.

Same goes for any other structure. If you called `init` then after use you must issue a `deinit`
on the same object. If you called `create` then you must explicitly issue a `destroy`.
