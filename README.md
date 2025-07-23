# SoundAgentLib (ApiClient + Common)

Multiplatform reusable C++ code for Sound Agents, supporting both Windows and Linux.
See [SoundLinuxAgent](https://github.com/eduarddanziger/SoundLinuxAgent.git) and [SoundWinAgent](https://github.com/eduarddanziger/SoundWinAgent.git) repositories.


## Features

- **HTTP Request Processing:** Asynchronous HTTP request handling using [cpprestsdk](https://github.com/microsoft/cpprestsdk) via own non-persistent quieue implementation.
- **Logging:** Asynchronous, dynamically configurable logging using [spdlog](https://github.com/gabime/spdlog), with support for rotating file logs and console output.
- **Cross-platform support:** Windows (`_WIN32`) and Linux (`__linux__`) compatibility.
- **C++20:** Modern C++ features for safety and performance.

## Prerequisites

- C++20 compatible compiler (e.g., MSVC, GCC, Clang)
- [spdlog](https://github.com/gabime/spdlog) library
