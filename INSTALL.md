## ⚙️ Berkeley DB 4.8 Requirement

LightningCash-R, like its predecessors (LightningCash, Litecoin Cash, Litecoin, Bitcoin), relies on **Berkeley DB 4.8** for wallet functionality. More recent versions of Berkeley DB (e.g. 5.x and 6.x) are **not compatible** with the legacy wallet format and may result in database corruption or wallet errors.

### Why Berkeley DB 4.8?

- Ensures compatibility with `wallet.dat` files used by Bitcoin-based clients.
- Prevents issues with wallet loading and synchronization.
- Avoids licensing complications introduced in later Berkeley DB versions.

### 📥 Installing Berkeley DB 4.8

You can install Berkeley DB 4.8 using the provided helper script located in the `contrib` directory:

```bash
cd contrib
./install_db4.sh `pwd`
````

This installs Berkeley DB 4.8 locally to the `contrib/db4` directory.

### 🔧 Setting Up the Build Environment

After installing Berkeley DB 4.8, you'll need to configure the build to use the correct path.

Run the following commands:

```bash
export BDB_PREFIX=$(pwd)/contrib/db4
./autogen.sh
./configure --with-incompatible-bdb --with-gui=no --prefix=$BDB_PREFIX --disable-tests
make
```

> If you are building the Qt wallet interface, set `--with-gui=yes` instead.

The `--with-incompatible-bdb` flag allows building with an older BDB version. The `--prefix` ensures the compiler and linker target the locally installed BDB 4.8, not a system-wide version.

---

## 🏗️ Building LightningCash-R

See the `doc/build-*.md` files for detailed platform-specific build instructions for the various components of the **LightningCash-R Core** reference implementation.

These documents include setup guidance for:

* Linux
* macOS
* Windows (via WSL or native)
* Dependency management and troubleshooting