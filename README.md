# SoundAgentLib (obsolete) -> ARCHIVED

- Currently used as a submodule by an obsolete monolith agent [SoundWinAgent](https://github.com/eduarddanziger/SoundWinAgent)
- The successors of sound agents are Windows and Linux sound scanners, that can be found in [Collect Sound Devices solution domain](https://github.com/collect-sound-devices)

## Features

- **HTTP Request publishing to RabbitMQ**
- **Logging:** Asynchronous, dynamically configurable logging using [spdlog](https://github.com/gabime/spdlog), with support for rotating file logs and console output.
- **Cross-platform support:** Windows (`_WIN32`) and Linux (`__linux__`) compatibility.
- **C++20:** Modern C++ features for safety and performance.

## Prerequisites

- C++20 compatible compiler (e.g., MSVC, GCC, Clang)
