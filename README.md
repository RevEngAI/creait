# C RevEngAI Toolkit (creait)

`creait` is a C library for interacting with the RevEngAI API. It provides a set of functions to perform binary analysis, decompilation, and function identification using RevEngAI's machine learning models.

## Features

- Binary analysis with AI-powered models
- Function identification and annotation
- AI-assisted decompilation
- Binary similarity search
- Collection management
- Function renaming and annotation

## Installation

### Prerequisites

Before building, ensure you have the following dependencies installed:

- libcurl (development package)
- git
- cmake
- make or ninja
- pkg-config

#### On Ubuntu/Debian:
```sh
sudo apt install libcurl4-openssl-dev git cmake ninja-build pkg-config
```

#### On macOS:
```sh
brew install cmake ninja curl pkg-config
```

#### On Windows:
- Install Visual Studio with C/C++ development tools
- Install Git from the official website
- Install Python and then run `pip install meson`
- Install pkg-config: `choco install pkgconfiglite` (requires Chocolatey)

### Building and Installing

```sh
# Clone this repo and cd into it
git clone git@github.com:RevEngAI/creait.git && cd creait

# Configure the build using ninja
cmake -B Build -G Ninja

# Build and install creait
ninja -C Build && sudo ninja -C Build install
```

For macOS users building cJSON from source:
```sh
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_NAME_DIR=/usr/local/lib
```

## Configuration System

The library includes a simple configuration system that allows users to store and retrieve key-value pairs. Configuration files use a simple format with one key-value pair per line, separated by an equals sign (`=`).

### Configuration File Format

```
api_key = your_api_key_here
host = https://api.reveng.ai
timeout = 30
debug = true
```

Each line contains a key-value pair in the format `key = value`. Whitespace around keys and values is automatically trimmed.

### Default Configuration Location

The default configuration file is located at:
- Windows: `%USERPROFILE%\.creait`
- macOS/Linux: `$HOME/.creait`

### Using the Configuration System

```c
#include <Reai/Config.h>

int main() {
    // Read configuration from the default location
    Config config = ConfigRead(NULL);
    
    // Or read from a specific path
    // Config config = ConfigRead("/path/to/config.ini");
    
    // Get a value from the configuration
    Str* api_key = ConfigGet(&config, "api_key");
    if (api_key) {
        printf("API Key: %s\n", api_key->data);
    } else {
        printf("API Key not found in config\n");
    }
    
    // Add or update a configuration value
    ConfigAdd(&config, "new_key", "new_value");
    
    // Write the configuration back to a file
    ConfigWrite(&config, NULL);  // Write to default location
    // Or write to a specific path
    // ConfigWrite(&config, "/path/to/config.ini");
    
    // Clean up
    ConfigDeinit(&config);
    
    return 0;
}
```

### Integration with API Functions

You can use the configuration system to store your API key and host information:

```c
#include <Reai/Api.h>
#include <Reai/Config.h>

int main() {
    // Read configuration
    Config config = ConfigRead(NULL);
    
    // Initialize connection
    Connection conn = ConnectionInit();
    
    // Set API key and host from config
    Str* api_key = ConfigGet(&config, "api_key");
    Str* host = ConfigGet(&config, "host");
    
    if (api_key && host) {
        StrCopy(&conn.api_key, api_key->data);
        StrCopy(&conn.host, host->data);
        
        // Authenticate
        if (Authenticate(&conn)) {
            printf("Authentication successful\n");
            // Proceed with API calls
        } else {
            printf("Authentication failed\n");
        }
    } else {
        printf("API key or host not found in config\n");
    }
    
    // Clean up
    ConfigDeinit(&config);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

## Working with Request Objects

The library provides convenient macros for initializing and cleaning up request objects. Always use these macros to ensure proper memory management.

### Initializing Request Objects

```c
// Initialize a new analysis request with default values
NewAnalysisRequest request = NewAnalysisRequestInit();

// Initialize a search request with default values
SearchBinaryRequest search_req = SearchBinaryRequestInit();

// Initialize a similar functions request
SimilarFunctionsRequest similar_req = SimilarFunctionsRequestInit();
```

### Cleaning Up Request Objects

```c
// Clean up a new analysis request
NewAnalysisRequestDeinit(&request);

// Clean up a search request
SearchBinaryRequestDeinit(&search_req);

// Clean up a similar functions request
SimilarFunctionsRequestDeinit(&similar_req);
```

## Status Handling

The library uses a unified status system for tracking the state of analyses, decompilations, and other operations. Status values include a type flag to indicate their source.

### Status Types

- `ANALYSIS_STATUS`: Status related to binary analysis
- `DYN_EXEC_STATUS`: Status related to dynamic execution
- `AI_DECOMP_STATUS`: Status related to AI decompilation

### Common Status Values

- `STATUS_QUEUED`: Operation is queued
- `STATUS_PROCESSING` / `STATUS_RUNNING` / `STATUS_PENDING`: Operation is in progress
- `STATUS_COMPLETE` / `STATUS_SUCCESS`: Operation completed successfully
- `STATUS_ERROR`: Operation failed
- `STATUS_UPLOADED`: Binary was uploaded but not yet processed

### Converting Between Status and String

```c
#include <Reai/Api.h>
#include <Reai/Api/Types/Status.h>

// Convert status to string
Status status = GetAnalysisStatus(&conn, binary_id);
Str status_str = StrInit();
StatusToStr(status, &status_str);
printf("Status: %s\n", status_str.data);
StrDeinit(&status_str);

// Convert string to status
Str status_str = StrInit();
StrCopy(&status_str, "Complete");
Status status = StatusFromStr(&status_str);
// Use STATUS_MASK to get the base status without flags
if ((status & STATUS_MASK) == STATUS_COMPLETE) {
    printf("Analysis is complete\n");
}
StrDeinit(&status_str);
```

## Usage Examples

### Authentication

Before using the API, you need to authenticate with your API key:

```c
#include <Reai/Api.h>

int main() {
    // Initialize connection
    Connection conn = ConnectionInit();
    
    // Set API key and host
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    printf("Authentication successful\n");
    
    // Clean up
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Uploading a Binary File

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Upload a binary file
    Str file_path = StrInit();
    StrCopy(&file_path, "/path/to/your/binary");
    
    Str sha256 = UploadFile(&conn, file_path);
    if (sha256.length == 0) {
        printf("File upload failed\n");
        StrDeinit(&file_path);
        return 1;
    }
    
    printf("File uploaded successfully. SHA256: %s\n", sha256.data);
    
    // Clean up
    StrDeinit(&file_path);
    StrDeinit(&sha256);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Creating a New Analysis

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Upload a binary file
    Str file_path = StrInit();
    StrCopy(&file_path, "/path/to/your/binary");
    
    Str sha256 = UploadFile(&conn, file_path);
    if (sha256.length == 0) {
        printf("File upload failed\n");
        return 1;
    }
    
    // Create a new analysis request using the initialization macro
    NewAnalysisRequest request = NewAnalysisRequestInit();
    
    // Set model name
    StrCopy(&request.ai_model, "binnet-v1");
    
    // Set file options
    request.file_opt = FILE_OPTION_AUTO;
    
    // Set file details
    StrCopy(&request.file_name, "example_binary");
    StrCopy(&request.sha256, sha256.data);
    
    request.file_size = 1024; // Replace with actual file size
    
    // Set analysis options
    request.dynamic_execution = true;
    request.is_private = true;
    request.priority = 1;
    
    // Submit analysis request
    BinaryId binary_id = CreateNewAnalysis(&conn, &request);
    if (binary_id == 0) {
        printf("Analysis creation failed\n");
        return 1;
    }
    
    printf("Analysis created successfully. Binary ID: %llu\n", binary_id);
    
    // Clean up using the deinitialization macro
    NewAnalysisRequestDeinit(&request);
    StrDeinit(&file_path);
    StrDeinit(&sha256);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Getting Analysis Status

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Get analysis status for a binary
    BinaryId binary_id = 123456; // Replace with your binary ID
    Status status = GetAnalysisStatus(&conn, binary_id);
    
    printf("Analysis status: ");
    switch (status & STATUS_MASK) {
        case STATUS_QUEUED:
            printf("Queued\n");
            break;
        case STATUS_PROCESSING:
            printf("Processing\n");
            break;
        case STATUS_COMPLETE:
            printf("Completed\n");
            break;
        case STATUS_ERROR:
            printf("Failed\n");
            break;
        default:
            printf("Unknown\n");
    }
    
    // Clean up
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Getting Function Information

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Get function information for a binary
    BinaryId binary_id = 123456; // Replace with your binary ID
    FunctionInfos functions = GetBasicFunctionInfoUsingBinaryId(&conn, binary_id);
    
    printf("Found %zu functions:\n", functions.size);
    for (size_t i = 0; i < functions.size; i++) {
        FunctionInfo* func = &functions.data[i];
        printf("Function ID: %llu, Name: %s, Address: 0x%llx, Size: %zu\n",
               func->id, func->symbol.name.data, func->symbol.value.addr, func->size);
    }
    
    // Clean up
    VecDeinit(&functions);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Renaming Functions

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Rename a function
    FunctionId function_id = 123456; // Replace with your function ID
    Str new_name = StrInit();
    StrCopy(&new_name, "process_user_input");
    
    if (RenameFunction(&conn, function_id, new_name)) {
        printf("Function renamed successfully\n");
    } else {
        printf("Function renaming failed\n");
    }
    
    // Clean up
    StrDeinit(&new_name);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### AI Decompilation

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Request AI decompilation for a function
    FunctionId function_id = 123456; // Replace with your function ID
    
    if (!BeginAiDecompilation(&conn, function_id)) {
        printf("Failed to start AI decompilation\n");
        return 1;
    }
    
    // Poll for decompilation status
    Status status;
    do {
        sleep(5); // Wait 5 seconds between checks
        status = GetAiDecompilationStatus(&conn, function_id);
        
        printf("Decompilation status: ");
        switch (status & STATUS_MASK) {
            case STATUS_QUEUED:
            case STATUS_UNINITIALIZED:
                printf("Queued\n");
                break;
            case STATUS_PROCESSING:
            case STATUS_RUNNING:
                printf("Processing\n");
                break;
            case STATUS_COMPLETE:
                printf("Completed\n");
                break;
            case STATUS_ERROR:
                printf("Failed\n");
                break;
            default:
                printf("Unknown\n");
        }
    } while ((status & STATUS_MASK) == STATUS_QUEUED || 
             (status & STATUS_MASK) == STATUS_PROCESSING ||
             (status & STATUS_MASK) == STATUS_RUNNING ||
             (status & STATUS_MASK) == STATUS_UNINITIALIZED);
    
    if ((status & STATUS_MASK) == STATUS_COMPLETE) {
        // Get decompilation result with AI summary
        AiDecompilation decompilation = GetAiDecompilation(&conn, function_id, true);
        
        printf("Decompiled code:\n%s\n\n", decompilation.decompiled_code.data);
        printf("AI Summary:\n%s\n", decompilation.ai_summary.data);
        
        // Clean up decompilation
        StrDeinit(&decompilation.decompiled_code);
        StrDeinit(&decompilation.ai_summary);
    }
    
    // Clean up
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Finding Similar Functions

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Create a request to find similar functions using the initialization macro
    SimilarFunctionsRequest request = SimilarFunctionsRequestInit();
    request.function_id = 123456; // Replace with your function ID
    request.limit = 10;
    request.distance = 0.8; // Similarity threshold (0.0 to 1.0)
    
    // Configure search options
    request.debug_include.user_symbols = true;
    request.debug_include.system_symbols = true;
    request.debug_include.external_symbols = true;
    
    // Get similar functions
    SimilarFunctions similar = GetSimilarFunctions(&conn, &request);
    
    printf("Found %zu similar functions:\n", similar.size);
    for (size_t i = 0; i < similar.size; i++) {
        SimilarFunction* func = &similar.data[i];
        printf("Function: %s, Distance: %f\n", func->function_name.data, func->distance);
    }
    
    // Clean up using the deinitialization macro
    SimilarFunctionsRequestDeinit(&request);
    VecDeinit(&similar);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

### Searching for Binaries

```c
#include <Reai/Api.h>

int main() {
    Connection conn = ConnectionInit();
    StrCopy(&conn.api_key, "your_api_key_here");
    StrCopy(&conn.host, "https://api.reveng.ai");
    
    // Authenticate
    if (!Authenticate(&conn)) {
        printf("Authentication failed\n");
        return 1;
    }
    
    // Create a search request using the initialization macro
    SearchBinaryRequest request = SearchBinaryRequestInit();
    request.page = 1;
    request.page_size = 10;
    
    StrCopy(&request.partial_name, "example");
    
    // Search for binaries
    BinaryInfos binaries = SearchBinary(&conn, &request);
    
    printf("Found %zu binaries:\n", binaries.size);
    for (size_t i = 0; i < binaries.size; i++) {
        BinaryInfo* binary = &binaries.data[i];
        printf("Binary ID: %llu, Name: %s\n", binary->id, binary->name.data);
    }
    
    // Clean up using the deinitialization macro
    SearchBinaryRequestDeinit(&request);
    VecDeinit(&binaries);
    StrDeinit(&conn.api_key);
    StrDeinit(&conn.host);
    
    return 0;
}
```

## API Reference

For a complete list of API functions and their parameters, please refer to the header file:
`Include/Reai/Api.h`

Key API functions include:

- `Authenticate`: Authenticate with the RevEngAI API
- `UploadFile`: Upload a binary file for analysis
- `CreateNewAnalysis`: Submit a new analysis request
- `GetAnalysisStatus`: Check the status of an analysis
- `GetBasicFunctionInfoUsingBinaryId`: Get function information for a binary
- `RenameFunction`: Rename a function
- `BeginAiDecompilation`: Start AI decompilation for a function
- `GetAiDecompilationStatus`: Check the status of a decompilation
- `GetAiDecompilation`: Get decompiled code and AI summary
- `GetSimilarFunctions`: Find functions similar to a given function
- `SearchBinary`: Search for binaries
- `SearchCollection`: Search for collections

## License

Copyright (c) RevEngAI. All Rights Reserved.
