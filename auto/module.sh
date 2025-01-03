if [ "$TCH_PLATFORM" != win32 ]; then

    # tch_feature="-Wl,-E switch"
    # tch_feature_name=
    # tch_feature_run=no
    # tch_feature_incs=
    # tch_feature_path=
    # tch_feature_libs=-Wl,-E
    # tch_feature_test=
    # . auto/feature.sh

    # if [ $tch_found = yes ]; then
    #     MAIN_LINK="-Wl,-E"
    # fi

#     if [ "$TCH_CC_NAME" = "ccc" ]; then
#         echo "checking for C99 variadic macros ... disabled"
#     else
#         tch_feature="C99 variadic macros"
#         tch_feature_name="TCH_HAVE_C99_VARIADIC_MACROS"
#         tch_feature_run=yes
#         tch_feature_incs="#include <stdio.h>
# #define var(dummy, ...)  sprintf(__VA_ARGS__)"
#         tch_feature_path=
#         tch_feature_libs=
#         tch_feature_test="char  buf[30]; buf[0] = '0';
#                           var(0, buf, \"%d\", 1);
#                           if (buf[0] != '1') return 1"
#         . auto/feature.sh
#     fi


    tch_feature="epoll variadic"
    tch_feature_name="HAVE_EPOLL"
    tch_feature_run=yes
    tch_feature_incs="#include <stdio.h>
                      #include <stdlib.h>
                      #include <sys/epoll.h>"
    tch_feature_path=
    tch_feature_libs=
    tch_feature_test="int fd;
                    fd = epoll_create(256);
                    if (fd < 0) {
                        exit(1);
                    }"
    . auto/feature.sh


    tch_feature="backtrace variadic"
    tch_feature_name="HAVE_BACKTRACE"
    tch_feature_run=yes
    tch_feature_incs="#include <execinfo.h>"
    tch_feature_path=
    tch_feature_libs=
    tch_feature_test="char  buf[30];"
    . auto/feature.sh

fi