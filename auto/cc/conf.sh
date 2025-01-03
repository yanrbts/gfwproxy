# Copyright (C) Yan Ruibing

#CC=${CC:-cc}
#CFLAGS="-pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g" 
#CFLAGS="-pipe  -O -W -Wall -Wpointer-arith  -g" 
#CPP="cc -E"
#LINK=$(CC)
LINK="\$(CC)"

MAIN_LINK=
MODULE_LINK="-shared"

tch_include_opt="-I "
tch_compile_opt="-c"
tch_pic_opt="-fPIC"
tch_objout="-o "
tch_binout="-o "
tch_objext="o"
tch_binext=
tch_modext=".so"

tch_long_start=
tch_long_end=

tch_regex_dirsep="\/"
tch_dirsep='/'

tch_regex_cont=' \\\
	'
tch_cont=' \
	'
tch_tab=' \
		'
tch_spacer=

tch_long_regex_cont=$tch_regex_cont
tch_long_cont=$tch_cont

. auto/cc/name.sh

if test -n "$CFLAGS"; then

    CC_TEST_FLAGS="$CFLAGS $TCH_CC_OPT"

    case $TCH_CC_NAME in

        ccc)
            # Compaq C V6.5-207

            tch_include_opt="-I"
        ;;

        sunc)

            MAIN_LINK=
            MODULE_LINK="-G"

            case "$TCH_MACHINE" in

                i86pc)
                   #NGX_AUX=" src/os/unix/ngx_sunpro_x86.il"
                ;;

                sun4u | sun4v)
                    #NGX_AUX=" src/os/unix/ngx_sunpro_sparc64.il"
                ;;

            esac

            case $CPU in

                amd64)
                    #NGX_AUX=" src/os/unix/ngx_sunpro_amd64.il"
                ;;

            esac
        ;;

    esac

else

    case $TCH_CC_NAME in
        gcc)
            # gcc 2.7.2.3, 2.8.1, 2.95.4, egcs-1.1.2
            #     3.0.4, 3.1.1, 3.2.3, 3.3.2, 3.3.3, 3.3.4, 3.4.0, 3.4.2
            #     4.0.0, 4.0.1, 4.1.0

            . auto/cc/gcc.sh
        ;;

        clang)
            # Clang C compiler

            #. auto/cc/clang
        ;;

        icc)
            # Intel C++ compiler 7.1, 8.0, 8.1

            #. auto/cc/icc
        ;;

        sunc)
            # Sun C 5.7 Patch 117837-04 2005/05/11

            #. auto/cc/sunc
        ;;

        ccc)
            # Compaq C V6.5-207

            #. auto/cc/ccc
        ;;

        acc)
            # aCC: HP ANSI C++ B3910B A.03.55.02

            #. auto/cc/acc
        ;;

        msvc*)
            # MSVC++ 6.0 SP2, MSVC++ Toolkit 2003

            #. auto/cc/msvc
        ;;

        owc)
            # Open Watcom C 1.0, 1.2

            #. auto/cc/owc
        ;;

        bcc)
            # Borland C++ 5.5

            #. auto/cc/bcc
        ;;

    esac

    CC_TEST_FLAGS="$CC_TEST_FLAGS $TCH_CC_OPT"

fi

CFLAGS="$CFLAGS $TCH_CC_OPT"
TCH_TEST_LD_OPT="$TCH_LD_OPT"

if [ "$TCH_PLATFORM" != win32 ]; then

    if test -n "$TCH_LD_OPT"; then
        tch_feature=--with-ld-opt=\"$TCH_LD_OPT\"
        tch_feature_name=
        tch_feature_run=no
        tch_feature_incs=
        tch_feature_path=
        tch_feature_libs=
        tch_feature_test=
        . auto/feature.sh

        if [ $tch_found = no ]; then
            echo $0: error: the invalid value in --with-ld-opt=\"$TCH_LD_OPT\"
            echo
            exit 1
        fi
    fi


    tch_feature="-Wl,-E switch"
    tch_feature_name=
    tch_feature_run=no
    tch_feature_incs=
    tch_feature_path=
    tch_feature_libs=-Wl,-E
    tch_feature_test=
    . auto/feature.sh

    if [ $tch_found = yes ]; then
        MAIN_LINK="-Wl,-E"
    fi


    if [ "$TCH_CC_NAME" = "sunc" ]; then
        echo "checking for gcc builtin atomic operations ... disabled"
    else
        tch_feature="gcc builtin atomic operations"
        tch_feature_name=TCH_HAVE_GCC_ATOMIC
        tch_feature_run=yes
        tch_feature_incs=
        tch_feature_path=
        tch_feature_libs=
        tch_feature_test="long  n = 0;
                          if (!__sync_bool_compare_and_swap(&n, 0, 1))
                              return 1;
                          if (__sync_fetch_and_add(&n, 1) != 1)
                              return 1;
                          if (n != 2)
                              return 1;
                          __sync_synchronize();"
        . auto/feature.sh
    fi


    if [ "$TCH_CC_NAME" = "ccc" ]; then
        echo "checking for C99 variadic macros ... disabled"
    else
        tch_feature="C99 variadic macros"
        tch_feature_name="TCH_HAVE_C99_VARIADIC_MACROS"
        tch_feature_run=yes
        tch_feature_incs="#include <stdio.h>
#define var(dummy, ...)  sprintf(__VA_ARGS__)"
        tch_feature_path=
        tch_feature_libs=
        tch_feature_test="char  buf[30]; buf[0] = '0';
                          var(0, buf, \"%d\", 1);
                          if (buf[0] != '1') return 1"
        . auto/feature.sh
    fi


    tch_feature="gcc variadic macros"
    tch_feature_name="TCH_HAVE_GCC_VARIADIC_MACROS"
    tch_feature_run=yes
    tch_feature_incs="#include <stdio.h>
#define var(dummy, args...)  sprintf(args)"
    tch_feature_path=
    tch_feature_libs=
    tch_feature_test="char  buf[30]; buf[0] = '0';
                      var(0, buf, \"%d\", 1);
                      if (buf[0] != '1') return 1"
    . auto/feature.sh


    tch_feature="gcc builtin 64 bit byteswap"
    tch_feature_name="TCH_HAVE_GCC_BSWAP64"
    tch_feature_run=no
    tch_feature_incs=
    tch_feature_path=
    tch_feature_libs=
    tch_feature_test="if (__builtin_bswap64(0)) return 1"
    . auto/feature.sh


#    ngx_feature="inline"
#    ngx_feature_name=
#    ngx_feature_run=no
#    ngx_feature_incs="int inline f(void) { return 1 }"
#    ngx_feature_path=
#    ngx_feature_libs=
#    ngx_feature_test=
#    . auto/feature
#
#    if [ $ngx_found = yes ]; then
#    fi

fi