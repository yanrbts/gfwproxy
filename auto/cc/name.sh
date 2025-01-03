# Copyright (C) Yan Ruibing

if [ "$TCH_PLATFORM" != win32 ]; then

    tch_feature="C compiler"
    tch_feature_name=
    tch_feature_run=yes
    tch_feature_incs=
    tch_feature_path=
    tch_feature_libs=
    tch_feature_test=
    . auto/feature.sh

    if [ $tch_found = no ]; then
        echo
        echo $0: error: C compiler $CC is not found
        echo
        exit 1
    fi

fi

if [ "$CC" = cl ]; then
    TCH_CC_NAME=msvc
    echo " + using Microsoft Visual C++ compiler"

elif [ "$CC" = wcl386 ]; then
    TCH_CC_NAME=owc
    echo " + using Open Watcom C compiler"

elif [ "$CC" = bcc32 ]; then
    TCH_CC_NAME=bcc
    echo " + using Borland C++ compiler"

elif `$CC -V 2>&1 | grep '^Intel(R) C' >/dev/null 2>&1`; then
    TCH_CC_NAME=icc
    echo " + using Intel C++ compiler"

elif `$CC -v 2>&1 | grep 'gcc version' >/dev/null 2>&1`; then
    TCH_CC_NAME=gcc
    echo " + using GNU C compiler"

elif `$CC -v 2>&1 | grep 'clang version' >/dev/null 2>&1`; then
    TCH_CC_NAME=clang
    echo " + using Clang C compiler"

elif `$CC -v 2>&1 | grep 'LLVM version' >/dev/null 2>&1`; then
    TCH_CC_NAME=clang
    echo " + using Clang C compiler"

elif `$CC -V 2>&1 | grep 'Sun C' >/dev/null 2>&1`; then
    TCH_CC_NAME=sunc
    echo " + using Sun C compiler"

elif `$CC -V 2>&1 | grep '^Compaq C' >/dev/null 2>&1`; then
    TCH_CC_NAME=ccc
    echo " + using Compaq C compiler"

elif `$CC -V 2>&1 | grep '^aCC: ' >/dev/null 2>&1`; then
    TCH_CC_NAME=acc
    echo " + using HP aC++ compiler"

else
    TCH_CC_NAME=unknown

fi