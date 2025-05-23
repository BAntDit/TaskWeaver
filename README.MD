
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

2. Clone project: `git clone git@github.com:BAntDit/TaskWeaver.git`

3. Install dependencies: `conan install . -s compiler.cppstd=20 -s build_type=<Debug|Release> --build=missing --output-folder=<Output folder>`

4. Update cmake cache: `cmake --preset <preset-name> -G Ninja`

5. Deploy to local conan cache: `conan create . -s build_type=<Debug|Release> -s compiler.cppstd=20`

## Version 

v0.3.0.0

[changelog](CHANGELOG.md)

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.