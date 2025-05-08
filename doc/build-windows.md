LightningCashr Core – Windows 11 Build Guide (WSL2)
===================================================

This updated guide focuses on building LightningCashr Core for Windows 11 using **WSL2 (Windows Subsystem for Linux)**, which offers improved performance and compatibility with newer Linux distributions such as Ubuntu 22.04 or later. GUI wallet support is not included in this guide.

---

Recommended Build Options
-------------------------
- **Primary (Recommended):** Windows 11 with WSL2 and Ubuntu 22.04+ using MinGW-w64 cross-compilation toolchain.
- **Alternative:** Native Windows build using MSYS2 or Visual Studio (not covered here).

---

Install WSL2 on Windows 11
--------------------------
1. Open PowerShell **as Administrator** and run:

    wsl --install

   This installs WSL2 and the default Ubuntu distribution (currently Ubuntu 22.04 or later).

2. After installation completes, restart your computer if prompted.

3. Open Ubuntu from the Start Menu and create a new UNIX user account.

---

Prepare Ubuntu Environment in WSL2
----------------------------------
Update and install dependencies:

    sudo apt update && sudo apt upgrade -y
    sudo apt install build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git

Install MinGW-w64 toolchain:

- For 64-bit builds:

    sudo apt install g++-mingw-w64-x86-64

- For 32-bit builds (optional):

    sudo apt install g++-mingw-w64-i686

Set the POSIX threading model (important for compatibility):

    sudo update-alternatives --config x86_64-w64-mingw32-g++
    # Choose the option that includes "posix"

(Optional) For 32-bit:

    sudo update-alternatives --config i686-w64-mingw32-g++

---

Important Notes for WSL2 Users
------------------------------
- Ensure the source code is **not under /mnt/c/** or any mounted Windows drive.
  Use a native Linux path like `/home/youruser/src/lightningcashr`.

  Example setup:

    mkdir -p ~/src
    cd ~/src
    git clone https://github.com/lightningcashr-project/lightningcashr.git
    cd lightningcashr

---

Build Instructions (64-bit)
---------------------------
1. Strip Windows paths from environment (avoids toolchain issues):

    export PATH=$(echo "$PATH" | sed -e 's/:\\/mnt.*//g')

2. Build dependencies:

    cd depends
    make HOST=x86_64-w64-mingw32
    cd ..

3. Configure build:

    ./autogen.sh
    CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=/

4. Compile:

    make -j$(nproc)

---

Build Instructions (32-bit)
---------------------------
Follow the same steps as above but substitute the host target:

    cd depends
    make HOST=i686-w64-mingw32
    cd ..
    CONFIG_SITE=$PWD/depends/i686-w64-mingw32/share/config.site ./configure --prefix=/
    make -j$(nproc)

---

Install and Export
------------------
To copy your build to a Windows-accessible folder:

    make install DESTDIR=/mnt/c/Users/YourUsername/Desktop/lightningcashr

You can now find your Windows executables under:
`C:\\Users\\YourUsername\\Desktop\\lightningcashr\\usr\\local\\bin`

---

Additional Tips
---------------
- Run all build commands inside WSL Ubuntu terminal.
- Avoid building in `/mnt/c/...` or other mounted drives.
- Use `make clean` and rebuild if switching between 32-bit and 64-bit targets.

---

Done!
-----
You have successfully built LightningCashr Core for Windows using WSL2 on Windows 11 🎉