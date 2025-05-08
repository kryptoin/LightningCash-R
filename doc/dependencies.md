# LightningCash-R Core – Modern Dependencies (2025 Edition)

This document lists the updated and recommended dependencies required to build and run LightningCash-R Core, a Bitcoin/Litecoin-derived altcoin. It includes core libraries, GUI components, and optional tools for development and packaging. These dependencies are suitable for modern systems including Windows 11, macOS (M1–M4), and recent Linux distributions.

## Core Dependencies

| Dependency     | Recommended Version | Minimum Required | CVEs Known | Purpose / Notes                      |
|----------------|---------------------|------------------|-------------|--------------------------------------|
| **Berkeley DB** | 4.8.30              | 4.8.x            | No          | Required for wallet support          |
| **Boost**       | ≥1.70               | 1.47.0           | No          | Core C++ utilities, threading, etc.  |
| **ccache**      | 4.8                 | Any              | No          | Speeds up repeated builds            |
| **Clang/GCC**   | Clang 14+ / GCC 11+ | 4.8+             | No          | C++11+ compiler support              |
| **libevent**    | 2.1.12              | 2.0.22           | No          | Event loop, networking               |
| **OpenSSL**     | 1.1.1 / 3.x         | 1.0.1+           | Yes         | RPC encryption and P2P TLS (optional)|
| **protobuf**    | 3.21+               | 2.6.1            | No          | Serialization for network/messages   |
| **Python**      | 3.10+               | 3.4              | No          | Test scripts and automation          |
| **zlib**        | 1.3                 | Any              | No          | Compression utilities                |

## GUI and Qt Dependencies (optional)

| Dependency     | Recommended Version | Minimum Required | CVEs Known | Purpose / Notes                      |
|----------------|---------------------|------------------|-------------|--------------------------------------|
| **Qt**          | 5.15.2 / 6.x        | 4.7               | No          | GUI interface for desktop wallets    |
| **D-Bus**       | 1.14+               | Any              | No          | Required by Qt on Linux              |
| **Expat**       | 2.5.0               | Any              | No          | XML parsing                          |
| **fontconfig**  | 2.14+               | Any              | No          | Font discovery                       |
| **FreeType**    | 2.13+               | Any              | No          | Font rendering                       |
| **HarfBuzz**    | 8.3+                | Any              | No          | Font shaping                         |
| **libjpeg**     | 9e                  | Any              | No          | Image support in Qt                  |
| **libpng**      | 1.6.40              | Any              | No          | Image support in Qt                  |
| **PCRE**        | 8.45                | Any              | No          | Regex in Qt-based UIs                |
| **XCB**         | 1.14+               | Any              | No          | Required for Qt GUI on Linux         |
| **xkbcommon**   | 1.5.0+              | Any              | No          | Keyboard input (Linux)               |

## Optional Features

| Dependency     | Recommended Version | Minimum Required | CVEs Known | Purpose / Notes                      |
|----------------|---------------------|------------------|-------------|--------------------------------------|
| **MiniUPnPc**   | 2.2.6               | Any              | No          | Automatic port forwarding            |
| **qrencode**    | 4.1.1               | Any              | No          | QR code generation                   |
| **ZeroMQ**      | 4.3.5               | Any              | No          | Pub/sub notifications via ZMQ        |
| **librsvg**     | Latest              | Any              | No          | .dmg image generation on macOS       |

## Build Tools and Utilities

- **autoconf**, **automake**, **libtool**: Required for `./autogen.sh` and building from source.
- **pkg-config**: Detects and configures compiler and linker flags.
- **git**, **curl**, **rsync**, **bsdmainutils**: Basic dev tools for fetching sources and scripts.

## Compatibility Notes

- Use **system shared libraries** whenever possible to reduce binary size and improve maintainability.
- Qt 6 is now supported by most Linux distributions and macOS (M1+), but check for compatibility with your toolchain.
- **Clang** is preferred on macOS and is compatible with Homebrew-based builds.

## Notes

- Always check for recent CVEs and library updates in production environments.
- Some libraries may need to be patched or configured differently based on your OS or architecture (e.g., M1, M2, M3, M4).
- Building with **depends/** system isolates dependencies and is recommended for deterministic builds.

---
_Last updated: May 2025_