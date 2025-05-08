MacDeploy (Updated for Apple Silicon, macOS 12+)

This script automates the packaging of a Qt-based LightningCash-R Core application into a .dmg file suitable for distribution on macOS.

Requirements

Before running `make deploy`, ensure you have the following installed:

1. Xcode Command Line Tools

   Run this command in your terminal:

       xcode-select --install

2. Homebrew (if not already installed):

       /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

3. Qt (version 5.15 or newer recommended)

   Install Qt using Homebrew:

       brew install qt@5

   If using Homebrew’s qt@5, update your PATH:

       export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"

   Or, if installed via the official Qt installer:

       export PATH="/path/to/Qt/5.15.2/clang_64/bin:$PATH"

4. macdeployqt

   This is included with Qt and is used to bundle the .app.

5. create-dmg

   This is a utility to create styled disk images:

       brew install create-dmg

Deploying the Application

Once you’ve built the LightningCash-R Core application using:

    make

You can create the .dmg file with:

    make deploy

This will:

- Use macdeployqt to bundle required Qt libraries into the .app
- Package the .app into a signed and styled .dmg
- Output the result as:

    LightningCashr-Core.dmg

During this process, a Finder window may briefly appear as styling is applied. This is expected — do not interfere.

Notes

- Supports macOS Monterey, Ventura, Sonoma, and newer
- Compatible with Apple Silicon (M1, M2, M3, M4) and Intel Macs (via Rosetta or universal builds)
- Requires that you build a GUI-enabled Qt application
- Signing and notarization must be handled manually or via automated CI pipelines