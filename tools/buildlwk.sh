#!/bin/bash
set -eox pipefail

REPO=https://github.com/Blockstream/lwk
COMMIT=5b015d6548a2e5ce60c5fcaae449248447e4c02d

mkdir -p build && cd build

if [ ! -d lwk-src ]; then
    git clone --recurse-submodules --quiet $REPO lwk-src
fi

cd lwk-src
git rev-parse HEAD
git checkout $COMMIT

if [ "$HOST" = "windows" ]; then
    cargo build --target x86_64-pc-windows-gnu --release -p lwk_bindings
    cd target/x86_64-pc-windows-gnu/release
    gendef lwk.dll
    install -d $PREFIX/bin
    install lwk.dll $PREFIX/bin
    install lwk.def $PREFIX/bin
elif [ "$HOST" = "macos" ]; then
    cargo build --release -p lwk_bindings
    install -d $PREFIX/lib
    install target/release/liblwk.dylib $PREFIX/lib
    install_name_tool -id @rpath/liblwk.dylib $PREFIX/lib/liblwk.dylib
else
    cargo build --release -p lwk_bindings
    install -d $PREFIX/lib
    install ./target/release/liblwk.so $PREFIX/lib
fi
