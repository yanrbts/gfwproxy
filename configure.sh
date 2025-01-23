#!/bin/sh
# Copyright (C) Yan Ruibing

LC_ALL=C
export LC_ALL

. auto/options.sh
. auto/init.sh
. auto/sources.sh

test -d $TCH_OBJS || mkdir -p $TCH_OBJS

echo "#define TCH_CONFIGURE \"$TCH_CONFIGURE\"" > $TCH_AUTO_CONFIG_H


if [ $TCH_DEBUG = YES ]; then
    have=GF_DEBUG_LOG . auto/write.sh
fi

if [ $TCH_STATS = YES ]; then
    have=HAVE_STATS . auto/write.sh
fi

if test -z "$TCH_PLATFORM"; then
    echo "checking for OS"

    TCH_SYSTEM=`uname -s 2>/dev/null`
    TCH_RELEASE=`uname -r 2>/dev/null`
    TCH_MACHINE=`uname -m 2>/dev/null`

    echo " + $TCH_SYSTEM $TCH_RELEASE $TCH_MACHINE"

    TCH_PLATFORM="$TCH_SYSTEM:$TCH_RELEASE:$TCH_MACHINE";

    case "$TCH_SYSTEM" in
        MINGW32_* | MINGW64_* | MSYS_*)
            TCH_PLATFORM=win32
        ;;
    esac

else
    echo "building for $TCH_PLATFORM"
    TCH_SYSTEM=$TCH_PLATFORM
fi

tar xvfz contrib/yaml-0.2.5.tar.gz -C contrib
cd contrib/yaml-0.2.5
./configure
make
cd ../../

. auto/cc/conf.sh
. auto/os/conf.sh
. auto/module.sh
. auto/make.sh
. auto/install.sh