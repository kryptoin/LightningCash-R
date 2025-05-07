#!/bin/sh
# Copyright (c) 2013-2016 The Bitcoin Core developers
# Copyright (c) 2025 The LightningCash-R Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

set -e

srcdir="$(dirname "$0")"
cd "$srcdir"

# Check for required Autotools components
echo "Checking for Autotools and other build tools..."

# Check for autoreconf
which autoreconf >/dev/null || { echo "Error: autoreconf not found. Please install autoconf."; exit 1; }
AUTOCONF_VERSION=$(autoreconf --version | head -n 1 | awk '{print $NF}')
echo "Found autoreconf version: ${AUTOCONF_VERSION}"

# Check for automake
which automake >/dev/null || { echo "Error: automake not found. Please install automake."; exit 1; }
AUTOMAKE_VERSION=$(automake --version | head -n 1 | awk '{print $NF}')
echo "Found automake version: ${AUTOMAKE_VERSION}"

# Check for libtoolize (handle glibtoolize on some systems)
LIBTOOLIZE_BIN=""
if which libtoolize >/dev/null; then
    LIBTOOLIZE_BIN="libtoolize"
elif which glibtoolize >/dev/null; then
    LIBTOOLIZE_BIN="glibtoolize"
else
    echo "Error: libtoolize or glibtoolize not found. Please install libtool."
    exit 1
fi
LIBTOOL_VERSION=$(${LIBTOOLIZE_BIN} --version | head -n 1 | awk '{print $NF}')
echo "Found ${LIBTOOLIZE_BIN} version: ${LIBTOOL_VERSION}"
export LIBTOOLIZE="${LIBTOOLIZE_BIN}" # Export LIBTOOLIZE for autoreconf

# Optional: Check for pkg-config (commonly used)
if which pkg-config >/dev/null; then
    PKGCONFIG_VERSION=$(pkg-config --version)
    echo "Found pkg-config version: ${PKGCONFIG_VERSION}"
else
    echo "Warning: pkg-config not found. Some dependencies might not be detected correctly."
fi

# Optional: Check for gettext (commonly used for internationalization)
if which autopoint >/dev/null; then
     GETTEXT_VERSION=$(autopoint --version | head -n 1 | awk '{print $NF}')
     echo "Found autopoint version: ${GETTEXT_VERSION}"
else
     echo "Warning: autopoint (from gettext) not found. Internationalization support might be incomplete."
fi

echo "Running autoreconf to generate build system files..."

autoreconf --install --force --warnings=all --verbose

echo "Autogen complete. You should now be able to run ./configure"