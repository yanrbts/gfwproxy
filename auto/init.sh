# Copyright (C) Yan Ruibing

TCH_MAKEFILE=$TCH_OBJS/Makefile

TCH_AUTO_HEADERS_H=$TCH_OBJS/gf_auto_headers.h
TCH_AUTO_CONFIG_H=$TCH_OBJS/gf_auto_config.h

TCH_AUTOTEST=$TCH_OBJS/autotest
TCH_AUTOCONF_ERR=$TCH_OBJS/autoconf.err

TCH_PCH=
TCH_USE_PCH=

# check the echo's "-n" option and "\c" capability

if echo "test\c" | grep c >/dev/null; then

    if echo -n test | grep n >/dev/null; then
        tch_n=
        tch_c=

    else
        tch_n=-n
        tch_c=
    fi

else
    tch_n=
    tch_c='\c'
fi

# create Makefile

cat << END > Makefile

default:	build

clean:
	rm -rf Makefile $TCH_OBJS
END