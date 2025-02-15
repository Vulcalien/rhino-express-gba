#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage: $0 <ROM path>"
    exit
fi

ROM_PATH="$1"
ROM_NAME=$(basename "$ROM_PATH")

RELEASE_DIR="release"

# build ROM
make clean
make res
make build-deps
make build

# create release directory and copy files into it
rm -rf "$RELEASE_DIR"
mkdir "$RELEASE_DIR"
cp "$ROM_PATH" COPYING README.md "$RELEASE_DIR"

cd "$RELEASE_DIR"

# add padding to the ROM
truncate -s %512K "$ROM_NAME"

# clone repository into 'source'
git clone $(git remote get-url origin) source
git -C source gc --aggressive --prune=now

# clone all submodules inside 'source'
git -C source submodule update --recursive --init
git -C source submodule foreach --recursive\
    "git gc --aggressive --prune=now"

# generate checksums
mkdir checksum
md5sum $ROM_NAME > checksum/rom.md5
find source -type f -exec md5sum "{}" \; > checksum/source.md5

# create .zip file
zip -r release.zip "$ROM_NAME" COPYING README.md source checksum
