# SoundAgentLib (ApiClient + Common)

Multiplatform reusable C++ code for Sound Agents, supporting both Windows and Linux.
See [SoundLinuxAgent](https://github.com/eduarddanziger/SoundLinuxAgent) and [SoundWinScanner](https://github.com/collect-sound-devices/sound-win-scanner) repositories.


## Features

- **HTTP Request Enqueing to RabbitMQ:** Publishing HTTP request to RabbitMQ
- **Logging:** Asynchronous, dynamically configurable logging using [spdlog](https://github.com/gabime/spdlog), with support for rotating file logs and console output.
- **Cross-platform support:** Windows (`_WIN32`) and Linux (`__linux__`) compatibility.
- **C++20:** Modern C++ features for safety and performance.

## Prerequisites

- C++20 compatible compiler (e.g., MSVC, GCC, Clang)
