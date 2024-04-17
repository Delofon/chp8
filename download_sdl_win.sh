#!/usr/bin/env sh

# download SDL2 2.30.2

mk ()
{
    mkdir -p $@
}
mkcd ()
{
    mk $1 && cd $1
}

SDL2_NAME="SDL2-2.30.2"
SDL2_LIB_NAME="sdl2-lib.zip"
SDL2_DEV_NAME="sdl2-dev.tar.gz"

# download
mkcd external
curl -o "$SDL2_LIB_NAME" -L 'https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-2.30.2-win32-x64.zip'
curl -o "$SDL2_DEV_NAME" -L 'https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-devel-2.30.2-mingw.tar.gz'

# unpack runtime
unzip "$SDL2_LIB_NAME"
mk ../build
ln -s "../external/SDL2.dll" "../build/SDL2.dll"

# unpack devel
tar -xvzf "$SDL2_DEV_NAME"
rm "$SDL2_DEV_NAME"
mk SDL2
cp -r "${SDL2_NAME}/x86_64-w64-mingw32/." ./SDL2/

if [ -d "$SDL2_NAME" ]
then
    rm -rf "${SDL2_NAME}"
else
    echo "no sdl2 folder?" >&2
fi

