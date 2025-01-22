# Copyright (C) Yan Ruibing

# gcc 2.7.2.3, 2.8.1, 2.95.4, egcs-1.1.2
#     3.0.4, 3.1.1, 3.2.3, 3.3.2, 3.3.3, 3.3.4, 3.4.0, 3.4.2
#     4.0.0, 4.0.1, 4.1.0


TCH_GCC_VER=`$CC -v 2>&1 | grep 'gcc version' 2>&1 \
                         | sed -e 's/^.* version \(.*\)/\1/'`

echo " + gcc version: $TCH_GCC_VER"

have=TCH_COMPILER value="\"gcc $TCH_GCC_VER\"" . auto/define.sh


# Solaris 7's /usr/ccs/bin/as does not support "-pipe"

CC_TEST_FLAGS="-pipe"

tch_feature="gcc -pipe switch"
tch_feature_name=
tch_feature_run=no
tch_feature_incs=
tch_feature_path=
tch_feature_libs=
tch_feature_test=
. auto/feature.sh

CC_TEST_FLAGS=

if [ $tch_found = yes ]; then
    PIPE="-pipe"
fi


case "$TCH_MACHINE" in

    sun4u | sun4v | sparc | sparc64 )
        # "-mcpu=v9" enables the "casa" assembler instruction
        CFLAGS="$CFLAGS -mcpu=v9"
    ;;

esac


# optimizations

#NGX_GCC_OPT="-O2"
#NGX_GCC_OPT="-Os"
TCH_GCC_OPT="-O"

#CFLAGS="$CFLAGS -fomit-frame-pointer"

case $CPU in
    pentium)
        # optimize for Pentium and Athlon
        CPU_OPT="-march=pentium"
        TCH_CPU_CACHE_LINE=32
    ;;

    pentiumpro | pentium3)
        # optimize for Pentium Pro, Pentium II and Pentium III
        CPU_OPT="-march=pentiumpro"
        TCH_CPU_CACHE_LINE=32
    ;;

    pentium4)
        # optimize for Pentium 4, gcc 3.x
        CPU_OPT="-march=pentium4"
        TCH_CPU_CACHE_LINE=128
    ;;

    athlon)
        # optimize for Athlon, gcc 3.x
        CPU_OPT="-march=athlon"
        TCH_CPU_CACHE_LINE=64
    ;;

    opteron)
        # optimize for Opteron, gcc 3.x
        CPU_OPT="-march=opteron"
        TCH_CPU_CACHE_LINE=64
    ;;

    sparc32)
        # build 32-bit UltraSparc binary
        CPU_OPT="-m32"
        CORE_LINK="$CORE_LINK -m32"
        TCH_CPU_CACHE_LINE=64
    ;;

    sparc64)
        # build 64-bit UltraSparc binary
        CPU_OPT="-m64"
        CORE_LINK="$CORE_LINK -m64"
        TCH_CPU_CACHE_LINE=64
    ;;

    ppc64)
        # build 64-bit PowerPC binary
        CPU_OPT="-m64"
        CPU_OPT="$CPU_OPT -falign-functions=32 -falign-labels=32"
        CPU_OPT="$CPU_OPT -falign-loops=32 -falign-jumps=32"
        CORE_LINK="$CORE_LINK -m64"
        TCH_CPU_CACHE_LINE=128
    ;;

esac

CC_AUX_FLAGS="$CC_AUX_FLAGS $CPU_OPT"

case "$TCH_GCC_VER" in
    2.7*)
        # batch build
        CPU_OPT=
    ;;
esac


CFLAGS="$CFLAGS $PIPE $CPU_OPT"

if [ ".$PCRE_OPT" = "." ]; then
    PCRE_OPT="-O2 -fomit-frame-pointer $PIPE $CPU_OPT"
else
    PCRE_OPT="$PCRE_OPT $PIPE"
fi

if [ ".$ZLIB_OPT" = "." ]; then
    ZLIB_OPT="-O2 -fomit-frame-pointer $PIPE $CPU_OPT"
else
    ZLIB_OPT="$ZLIB_OPT $PIPE"
fi


# warnings

# -W requires at least -O
CFLAGS="$CFLAGS ${TCH_GCC_OPT:--O} -W"

CFLAGS="$CFLAGS -Wall -Wpointer-arith"
#CFLAGS="$CFLAGS -Wconversion"
#CFLAGS="$CFLAGS -Winline"
#CFLAGS="$CFLAGS -Wmissing-prototypes"

case "$TCH_GCC_VER" in
    2.*)
        # we have a lot of the unused function arguments
        CFLAGS="$CFLAGS -Wno-unused"
    ;;

    *)
        # we have a lot of the unused function arguments
        CFLAGS="$CFLAGS -Wno-unused-parameter -Wno-implicit-fallthrough"
        # 4.2.1 shows the warning in wrong places
        #CFLAGS="$CFLAGS -Wunreachable-code"

        # deprecated system OpenSSL library on OS X
        if [ "$TCH_SYSTEM" = "Darwin" ]; then
            CFLAGS="$CFLAGS -Wno-deprecated-declarations"
        fi
    ;;
esac


# stop on warning
CFLAGS="$CFLAGS -Werror"

# debug
CFLAGS="$CFLAGS -g"

# DragonFly's gcc3 generates DWARF
#CFLAGS="$CFLAGS -g -gstabs"

if [ ".$CPP" = "." ]; then
    CPP="$CC -E"
fi
