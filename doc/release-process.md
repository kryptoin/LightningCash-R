Release Process
====================

Before every release candidate:

* Update translations (ping wumpus on IRC) see [translation_process.md](https://github.com/bitcoin/bitcoin/blob/master/doc/translation_process.md#synchronising-translations).

* Update manpages, see [gen-manpages.sh](https://github.com/lightningcashr-project/lightningcashr/blob/master/contrib/devtools/README.md#gen-manpagessh).

Before every minor and major release:

* Update [bips.md](bips.md) to account for changes since the last release.
* Update version in `configure.ac` (don't forget to set `CLIENT_VERSION_IS_RELEASE` to `true`)
* Write release notes (see below)
* Update `src/chainparams.cpp` nMinimumChainWork with information from the getblockchaininfo rpc.
* Update `src/chainparams.cpp` defaultAssumeValid  with information from the getblockhash rpc.
  - The selected value must not be orphaned so it may be useful to set the value two blocks back from the tip.
  - Testnet should be set some tens of thousands back from the tip due to reorgs there.
  - This update should be reviewed with a reindex-chainstate with assumevalid=0 to catch any defect
     that causes rejection of blocks in the past history.

Before every major release:

* Update hardcoded [seeds](/contrib/seeds/README.md), see [this pull request](https://github.com/bitcoin/bitcoin/pull/7415) for an example.
* Update [`BLOCK_CHAIN_SIZE`](/src/qt/intro.cpp) to the current size plus some overhead.
* Update `src/chainparams.cpp` chainTxData with statistics about the transaction count and rate.
* Update version of `contrib/gitian-descriptors/*.yml`: usually one'd want to do this on master after branching off the release - but be sure to at least do it before a new major release

### First time / New builders

If you're using the automated script (found in [contrib/gitian-build.sh](/contrib/gitian-build.sh)), then at this point you should run it with the "--setup" command. Otherwise ignore this.

Check out the source code in the following directory hierarchy.

    cd /path/to/your/toplevel/build
    git clone https://github.com/lightningcashr-project/gitian.sigs.lcc.git
    git clone https://github.com/lightningcashr-project/lightningcashr-detached-sigs.git
    git clone https://github.com/devrandom/gitian-builder.git
    git clone https://github.com/lightningcashr-project/lightningcashr.git

### LightningCashr maintainers/release engineers, suggestion for writing release notes

Write release notes. git shortlog helps a lot, for example:

    git shortlog --no-merges v(current version, e.g. 0.7.2)..v(new version, e.g. 0.8.0)

(or ping @wumpus on IRC, he has specific tooling to generate the list of merged pulls
and sort them into categories based on labels)

Generate list of authors:

    git log --format='%aN' "$*" | sort -ui | sed -e 's/^/- /'

Tag version (or release candidate) in git

    git tag -s v(new version, e.g. 0.8.0)

### Setup and perform Gitian builds

If you're using the automated script (found in [contrib/gitian-build.sh](/contrib/gitian-build.sh)), then at this point you should run it with the "--build" command. Otherwise ignore this.

Setup Gitian descriptors:

    pushd ./lightningcashr
    export SIGNER=(your Gitian key, ie bluematt, sipa, etc)
    export VERSION=(new version, e.g. 0.8.0)
    git fetch
    git checkout v${VERSION}
    popd

Ensure your gitian.sigs.lcc are up-to-date if you wish to gverify your builds against other Gitian signatures.

    pushd ./gitian.sigs.lcc
    git pull
    popd

Ensure gitian-builder is up-to-date:

    pushd ./gitian-builder
    git pull
    popd

### Fetch and create inputs: (first time, or when dependency versions change)

    pushd ./gitian-builder
    mkdir -p inputs
    wget -P inputs https://bitcoincore.org/cfields/osslsigncode-Backports-to-1.7.1.patch
    wget -P inputs http://downloads.sourceforge.net/project/osslsigncode/osslsigncode/osslsigncode-1.7.1.tar.gz
    popd

Create the OS X SDK tarball, see the [OS X readme](README_osx.md) for details, and copy it into the inputs directory.

### Optional: Seed the Gitian sources cache and offline git repositories

By default, Gitian will fetch source files as needed. To cache them ahead of time:

    pushd ./gitian-builder
    make -C ../lightningcashr/depends download SOURCES_PATH=`pwd`/cache/common
    popd

Only missing files will be fetched, so this is safe to re-run for each build.

NOTE: Offline builds must use the --url flag to ensure Gitian fetches only from local URLs. For example:

    pushd ./gitian-builder
    ./bin/gbuild --url lightningcashr=/path/to/lightningcashr,signature=/path/to/sigs {rest of arguments}
    popd

The gbuild invocations below <b>DO NOT DO THIS</b> by default.

### Build and sign LightningCash-R Core for Linux, Windows, and OS X:

    pushd ./gitian-builder
    ./bin/gbuild --num-make 2 --memory 3000 --commit lightningcashr=v${VERSION} ../lightningcashr/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-linux --destination ../gitian.sigs.lcc/ ../lightningcashr/contrib/gitian-descriptors/gitian-linux.yml
    mv build/out/lightningcashr-*.tar.gz build/out/src/lightningcashr-*.tar.gz ../

    ./bin/gbuild --num-make 2 --memory 3000 --commit lightningcashr=v${VERSION} ../lightningcashr/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-win-unsigned --destination ../gitian.sigs.lcc/ ../lightningcashr/contrib/gitian-descriptors/gitian-win.yml
    mv build/out/lightningcashr-*-win-unsigned.tar.gz inputs/lightningcashr-win-unsigned.tar.gz
    mv build/out/lightningcashr-*.zip build/out/lightningcashr-*.exe ../

    ./bin/gbuild --num-make 2 --memory 3000 --commit lightningcashr=v${VERSION} ../lightningcashr/contrib/gitian-descriptors/gitian-osx.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-osx-unsigned --destination ../gitian.sigs.lcc/ ../lightningcashr/contrib/gitian-descriptors/gitian-osx.yml
    mv build/out/lightningcashr-*-osx-unsigned.tar.gz inputs/lightningcashr-osx-unsigned.tar.gz
    mv build/out/lightningcashr-*.tar.gz build/out/lightningcashr-*.dmg ../
    popd

Build output expected:

  1. source tarball (`lightningcashr-${VERSION}.tar.gz`)
  2. linux 32-bit and 64-bit dist tarballs (`lightningcashr-${VERSION}-linux[32|64].tar.gz`)
  3. windows 32-bit and 64-bit unsigned installers and dist zips (`lightningcashr-${VERSION}-win[32|64]-setup-unsigned.exe`, `lightningcashr-${VERSION}-win[32|64].zip`)
  4. OS X unsigned installer and dist tarball (`lightningcashr-${VERSION}-osx-unsigned.dmg`, `lightningcashr-${VERSION}-osx64.tar.gz`)
  5. Gitian signatures (in `gitian.sigs.lcc/${VERSION}-<linux|{win,osx}-unsigned>/(your Gitian key)/`)

### Verify other gitian builders signatures to your own. (Optional)

Add other gitian builders keys to your gpg keyring, and/or refresh keys.

    gpg --import lightningcashr/contrib/gitian-keys/*.pgp
    gpg --refresh-keys

Verify the signatures

    pushd ./gitian-builder
    ./bin/gverify -v -d ../gitian.sigs.lcc/ -r ${VERSION}-linux ../lightningcashr/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gverify -v -d ../gitian.sigs.lcc/ -r ${VERSION}-win-unsigned ../lightningcashr/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gverify -v -d ../gitian.sigs.lcc/ -r ${VERSION}-osx-unsigned ../lightningcashr/contrib/gitian-descriptors/gitian-osx.yml
    popd

### Next steps:

Commit your signature to gitian.sigs.lcc:

    pushd gitian.sigs.lcc
    git add ${VERSION}-linux/${SIGNER}
    git add ${VERSION}-win-unsigned/${SIGNER}
    git add ${VERSION}-osx-unsigned/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs.lcc tree
    popd

Codesigner only: Create Windows/OS X detached signatures:
- Only one person handles codesigning. Everyone else should skip to the next step.
- Only once the Windows/OS X builds each have 3 matching signatures may they be signed with their respective release keys.

Codesigner only: Sign the osx binary:

    transfer lightningcashr-osx-unsigned.tar.gz to osx for signing
    tar xf lightningcashr-osx-unsigned.tar.gz
    ./detached-sig-create.sh -s "Key ID"
    Enter the keychain password and authorize the signature
    Move signature-osx.tar.gz back to the gitian host

Codesigner only: Sign the windows binaries:

    tar xf lightningcashr-win-unsigned.tar.gz
    ./detached-sig-create.sh -key /path/to/codesign.key
    Enter the passphrase for the key when prompted
    signature-win.tar.gz will be created

Codesigner only: Commit the detached codesign payloads:

    cd ~/lightningcashr-detached-sigs
    checkout the appropriate branch for this release series
    rm -rf *
    tar xf signature-osx.tar.gz
    tar xf signature-win.tar.gz
    git add -a
    git commit -m "point to ${VERSION}"
    git tag -s v${VERSION} HEAD
    git push the current branch and new tag

Non-codesigners: wait for Windows/OS X detached signatures:

- Once the Windows/OS X builds each have 3 matching signatures, they will be signed with their respective release keys.
- Detached signatures will then be committed to the [lightningcashr-detached-sigs](https://github.com/lightningcashr-project/lightningcashr-detached-sigs) repository, which can be combined with the unsigned apps to create signed binaries.

Create (and optionally verify) the signed OS X binary:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../lightningcashr/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-osx-signed --destination ../gitian.sigs.lcc/ ../lightningcashr/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gverify -v -d ../gitian.sigs.lcc/ -r ${VERSION}-osx-signed ../lightningcashr/contrib/gitian-descriptors/gitian-osx-signer.yml
    mv build/out/lightningcashr-osx-signed.dmg ../lightningcashr-${VERSION}-osx.dmg
    popd

Create (and optionally verify) the signed Windows binaries:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../lightningcashr/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-win-signed --destination ../gitian.sigs.lcc/ ../lightningcashr/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gverify -v -d ../gitian.sigs.lcc/ -r ${VERSION}-win-signed ../lightningcashr/contrib/gitian-descriptors/gitian-win-signer.yml
    mv build/out/lightningcashr-*win64-setup.exe ../lightningcashr-${VERSION}-win64-setup.exe
    mv build/out/lightningcashr-*win32-setup.exe ../lightningcashr-${VERSION}-win32-setup.exe
    popd

Commit your signature for the signed OS X/Windows binaries:

    pushd gitian.sigs.lcc
    git add ${VERSION}-osx-signed/${SIGNER}
    git add ${VERSION}-win-signed/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs.lcc tree
    popd

### After 3 or more people have gitian-built and their results match:

- Create `SHA256SUMS.asc` for the builds, and GPG-sign it:

```bash
sha256sum * > SHA256SUMS
```

The list of files should be:
```
lightningcashr-${VERSION}-aarch64-linux-gnu.tar.gz
lightningcashr-${VERSION}-arm-linux-gnueabihf.tar.gz
lightningcashr-${VERSION}-i686-pc-linux-gnu.tar.gz
lightningcashr-${VERSION}-x86_64-linux-gnu.tar.gz
lightningcashr-${VERSION}-osx64.tar.gz
lightningcashr-${VERSION}-osx.dmg
lightningcashr-${VERSION}.tar.gz
lightningcashr-${VERSION}-win32-setup.exe
lightningcashr-${VERSION}-win32.zip
lightningcashr-${VERSION}-win64-setup.exe
lightningcashr-${VERSION}-win64.zip
```
The `*-debug*` files generated by the gitian build contain debug symbols
for troubleshooting by developers. It is assumed that anyone that is interested
in debugging can run gitian to generate the files for themselves. To avoid
end-user confusion about which file to pick, as well as save storage
space *do not upload these to the lightningca.sh server, nor put them in the torrent*.

- GPG-sign it, delete the unsigned file:
```
gpg --digest-algo sha256 --clearsign SHA256SUMS # outputs SHA256SUMS.asc
rm SHA256SUMS
```
(the digest algorithm is forced to sha256 to avoid confusion of the `Hash:` header that GPG adds with the SHA256 used for the files)
Note: check that SHA256SUMS itself doesn't end up in SHA256SUMS, which is a spurious/nonsensical entry.

- Upload zips and installers, as well as `SHA256SUMS.asc` from last step, to the lightningca.sh server.

```

- Update lightningca.sh version

- Announce the release:

  - lightningcashr-dev and lightningcashr-dev mailing list

  - blog.lightningca.sh blog post

  - Update title of #lightningcashr and #lightningcashr-dev on Freenode IRC

  - Optionally twitter, reddit /r/LightningCashr, ... but this will usually sort out itself

  - Archive release notes for the new version to `doc/release-notes/` (branch `master` and branch of the release)

  - Create a [new GitHub release](https://github.com/lightningcashr-project/lightningcashr/releases/new) with a link to the archived release notes.

  - Celebrate