# C RevEngAI Toolkit

Following are some helper guidelines to help users and developers navigate through code more naturally. Having read the following conventions, you'll
have a better idea of how the code is structured.

## Directory Structure

- All headers are in `Include/Reai` and when headers are installed they go to some place like `/usr/include/Reai` or `/usr/local/include/Reai` or any other `<PREFIX>/Reai`
  depending on where you're installing.
- All sources exist in `Source/Reai`
- Some headers in `Include/Reai/` are custom structures for which there also exist an array. These arrays are used to store information when the JSON in response to a request
  returns an array. Usage of these structures and corresponding vectors can be found in `Include/Reai/Api/Request.h` and `Source/Reai/Api/Request.c`.
- Headers like `Config.h` and `Log.h` don't have any vector representations and are there for easy loading of config file and creating logs in plugins. There are several macros
  to issue different levels of log severity messages.
- There are headers like `Types.h` and `Common.h` which are there for general types and macro definitions for your use.

## Adding New API Endpoints

Adding new endpoints is an easy 5 step process

1. Add structures required to make requests and get responses in `Include/Reai/Api/Request.h` and `Include/Reai/Api/Response.h` respectively.
   Each request and response type has it's own struct, present in a tagged union. For each corresponding request and response type,
   there's a matching enum type for this tagged enum.
   Whenever user has to make a RevEngAI request, they fill in the details, or use a helper method in `Include/Reai/Api/Reai.h` (defined by you)
   to ease the request process. The structure of these request and response objects are very much similar to what's displayed in the [API specification](https://api.reveng.ai/docs)
2. If any structure is required for which there must be an array type also, you should take references though header files present in `Include/Reai` folder.
   Define your structure (say `MyQueryResult`), define methods to init clone and deinit clones, and then make use of the macro `REAI_MAKE_VEC` to
   automatically define corresponding vector type, functions etc...
3. Implement cases to convert object data to JSON data in `Source/Reai/Api/Request.c` and to convert from JSON to object in `Source/Reai/Api/Response.c`.
   In some cases, you might have to convert object data to path query parameters. These cases need to be handled in `Source/Reai/Api/Reai.c` in `reai_request`
   method itself. Take references from already existing cases. Many cases look similar. Compare the [API docs](https://api.reveng.ai/docs) with implemented code to understand better.
4. Add endpoint request handler in `Source/Reai/Api/Reai.c` `reai_request` method. Each endpoint has it's own dedicated case.
   Take use of the helper macros to make your life easier.
5. Finally implement a deinit method for the response structure to deallocate and release all held memory and resources. Whenever a new request is made, this response
   struct is reset. Not doing so will result in memory leaks.

## Conventions

### Code Formatting.

There already exists a well defined `.clang-format` file in project source. Make sure you use a text editor that supports clang-format. If you prefer using simple nodepad,
then you must issue clang-format on all files through the command line. Please take help of your preferred search engine on how to do that. Using clang-format allows you
to not worry about code formatting, and just focus on actual coding. Many modern editors, even neovim and vim support plugins that can autoformat code on save using clang-format.

### Naming Conventions

Functions generally have a prefix (usually `reai_`), then the entity they interact with and then finally what they do as suffix. Some examples are

- `reai_cstr_vec_create(...)` : Interacting with a vector of `CString` (a.k.a `const char*`) to create a new vector.
- `reai_request(...)` : Interacting with `Reai` object to perform a request.
- `reai_response_init_for_type(...)` : Interacting with a `ReaiResponse` object to initialize response object from given JSON data for a specific type.
- `reai_auth_check(...)` : A helper function interacting with `Reai` object to make a request to perform authorization check (request to an endpoint to verify API ID).

### Calling Conventions

Functions take the object they interact with as first parameter, and return the object itself in most cases, or either return `true` or `false`. This is to make sure
the user function always gets to know of a failure.
