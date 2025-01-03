# Copyright (C) Yan Ruibing

echo $tch_n "checking for $tch_feature ...$tch_c"

cat << END >> $TCH_AUTOCONF_ERR

----------------------------------------
checking for $tch_feature

END

tch_found=no

if test -n "$tch_feature_name"; then
    tch_have_feature=`echo $tch_feature_name \
                   | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
fi

if test -n "$tch_feature_path"; then
    for tch_temp in $tch_feature_path; do
        tch_feature_inc_path="$tch_feature_inc_path -I $tch_temp"
    done
fi

cat << END > $TCH_AUTOTEST.c

#include <sys/types.h>
$TCH_INCLUDE_UNISTD_H
$tch_feature_incs

int main(void) {
    $tch_feature_test;
    return 0;
}

END

tch_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS $tch_feature_inc_path \
          -o $TCH_AUTOTEST $TCH_AUTOTEST.c $TCH_TEST_LD_OPT $tch_feature_libs"

tch_feature_inc_path=

eval "/bin/sh -c \"$tch_test\" >> $TCH_AUTOCONF_ERR 2>&1"

if [ -x $TCH_AUTOTEST ]; then

    case "$tch_feature_run" in

        yes)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $TCH_AUTOTEST >> $TCH_AUTOCONF_ERR 2>&1; then
                echo " found"
                tch_found=yes

                if test -n "$tch_feature_name"; then
                    have=$tch_have_feature . auto/write.sh
                fi

            else
                echo " found but is not working"
            fi
        ;;

        value)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $TCH_AUTOTEST >> $TCH_AUTOCONF_ERR 2>&1; then
                echo " found"
                tch_found=yes

                cat << END >> $TCH_AUTO_CONFIG_H

#ifndef $tch_feature_name
#define $tch_feature_name  `$TCH_AUTOTEST`
#endif

END
            else
                echo " found but is not working"
            fi
        ;;

        bug)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $TCH_AUTOTEST >> $TCH_AUTOCONF_ERR 2>&1; then
                echo " not found"

            else
                echo " found"
                tch_found=yes

                if test -n "$tch_feature_name"; then
                    have=$tch_have_feature . auto/write.sh
                fi
            fi
        ;;

        *)
            echo " found"
            tch_found=yes

            if test -n "$tch_feature_name"; then
                have=$tch_have_feature . auto/write.sh
            fi
        ;;

    esac

else
    echo " not found"

    echo "----------"    >> $TCH_AUTOCONF_ERR
    cat $TCH_AUTOTEST.c  >> $TCH_AUTOCONF_ERR
    echo "----------"    >> $TCH_AUTOCONF_ERR
    echo $tch_test       >> $TCH_AUTOCONF_ERR
    echo "----------"    >> $TCH_AUTOCONF_ERR
fi

rm -rf $TCH_AUTOTEST*
