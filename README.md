# C RevEngAI Toolkit

`creait` is a library for writing C applications that interact with the RevEngAI API.
creait is currently under development.

## Installation

```sh
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

When building and installing cJSON from source on Mac OS based systems, add `CMAKE_INSTALL_NAME_DIR=<install_prefix>/lib`
Where `<install_prefix>` is the prefix path you chose to install the library. This can be something like `/usr` or `/usr/local`
or if you're installing it some other place then that directory path. For example, my command looks like this :
`cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_NAME_DIR=/usr/local/lib`

On Windows

- You need Visual Studio.
- Git can be manually installed by downloading the installation setup.
- Python must be installed and then use `pip install meson` to install meson.
- pkg-config can be installed by running `choco install pkgconfiglite` after installing chocolatey package manager for windows.
