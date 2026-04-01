# SoundAgentLib (ApiClient + Common)

Multiplatform reusable C++ code for Sound Agents, supporting both Windows and Linux.
See [LinuxSoundScanner](https://github.com/collect-sound-devices/linux-sound-scanner) and [WinSoundScanner](https://github.com/collect-sound-devices/win-sound-scanner-go) repositories.


## Features

- **HTTP Request Enqueing to RabbitMQ:** Publishing HTTP request to RabbitMQ
- **Logging:** Asynchronous, dynamically configurable logging using [spdlog](https://github.com/gabime/spdlog), with support for rotating file logs and console output.
- **Cross-platform support:** Windows (`_WIN32`) and Linux (`__linux__`) compatibility.
- **C++20:** Modern C++ features for safety and performance.

## Prerequisites

- C++20 compatible compiler (e.g., MSVC, GCC, Clang)
