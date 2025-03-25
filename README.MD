
# TaskWeaver

A multithreaded task scheduler utilizing lock-free task-stealing deques.

## Build dependencies
[CONAN](https://conan.io/) - The open source, decentralized and multi-platform package
manager to create and share all your native binaries.

## Dependencies

1. [GTest](https://github.com/google/googletest) - Google Testing and Mocking Framework
2. [Metrix](https://github.com/BAntDit/easy-mp) - Meta-programming tricks library

## Project setup 

1. Install conan: [Installation instruction](https://docs.conan.io/2/installation.html)

2. Install metrix: 
    - `git clone git@github.com:BAntDit/easy-mp.git` - clone
    - `conan install . -s build_type=Release --build=missing --output-folder=cmake-build-release` - install
    - `sudo make install`

3. Setup project: `conan install . -s build_type=<Debug|Release> --build=missing --output-folder=<Output folder>`

## Version 

v0.3.0.0

[changelog](CHANGELOG.md)
