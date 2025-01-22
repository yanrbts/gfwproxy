# Copyright (C) Yan Ruibing

echo "creating $TCH_MAKEFILE"

mkdir -p $TCH_OBJS/src
mkdir -p $TCH_OBJS/src/event
mkdir -p $TCH_OBJS/src/hashkit

tch_objs_dir=$TCH_OBJS$tch_regex_dirsep
tch_use_pch=`echo $TCH_USE_PCH | sed -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                     > $TCH_MAKEFILE

CC =	$CC
CFLAGS = $CFLAGS
CPP =	$CPP
LINK =	$LINK

END

# ALL_INCS, required by the addons and by OpenWatcom C precompiled headers

tch_incs=`echo $CORE_INCS $TCH_OBJS\
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont$tch_include_opt\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $TCH_MAKEFILE

ALL_INCS = $tch_include_opt$tch_incs

END

tch_all_srcs="$CORE_SRCS"


# the core dependencies and include paths

tch_deps=`echo $CORE_DEPS $TCH_AUTO_CONFIG_H $TCH_PCH \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

tch_incs=`echo $CORE_INCS $TCH_OBJS \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont$tch_include_opt\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $TCH_MAKEFILE

CORE_DEPS = $tch_deps


CORE_INCS = $tch_include_opt$tch_incs

END

tch_all_srcs=`echo $tch_all_srcs | sed -e "s/\//$tch_regex_dirsep/g"`

for tch_src in $TCH_ADDON_SRCS
do
    tch_obj="addon/`basename \`dirname $tch_src\``"

    test -d $TCH_OBJS/$tch_obj || mkdir -p $TCH_OBJS/$tch_obj

    tch_obj=`echo $tch_obj/\`basename $tch_src\` \
        | sed -e "s/\//$tch_regex_dirsep/g"`

    tch_all_srcs="$tch_all_srcs $tch_obj"
done

tch_all_objs=`echo $tch_all_srcs \
    | sed -e "s#\([^ ]*\.\)cpp#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)cc#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)c#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)S#$TCH_OBJS\/\1$tch_objext#g"`

if test -n "$TCH_RES"; then
   tch_res=$TCH_RES
else
   tch_res="$TCH_RC $TCH_ICONS"
   tch_rcc=`echo $TCH_RCC | sed -e "s/\//$tch_regex_dirsep/g"`
fi

tch_deps=`echo $tch_all_objs $tch_res \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

tch_objs=`echo $tch_all_objs \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_long_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $TCH_MAKEFILE

build:	binary

binary:	$TCH_OBJS${tch_dirsep}gfw$tch_binext

$TCH_OBJS${tch_dirsep}gfw$tch_binext: $tch_deps$tch_spacer
	\$(LINK) $tch_long_start$tch_binout$TCH_OBJS${tch_dirsep}gfw$tch_binext$tch_long_cont$tch_objs$TCH_LIB
	$tch_rcc
$tch_long_end

modules:

END

if test -n "$TCH_PCH"; then
    tch_cc="\$(CC) $tch_compile_opt \$(CFLAGS) $tch_use_pch \$(ALL_INCS)"
else
    tch_cc="\$(CC) $tch_compile_opt \$(CFLAGS) \$(CORE_INCS)"
fi

# the core sources

for tch_src in $CORE_SRCS
do
    tch_src=`echo $tch_src | sed -e "s/\//$tch_regex_dirsep/g"`
    tch_obj=`echo $tch_src \
        | sed -e "s#^\(.*\.\)cpp\\$#$tch_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)cc\\$#$tch_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)c\\$#$tch_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)S\\$#$tch_objs_dir\1$tch_objext#g"`

    cat << END                                                >> $TCH_MAKEFILE

$tch_obj:	\$(CORE_DEPS)$tch_cont$tch_src
	$tch_cc$tch_tab$tch_objout$tch_obj$tch_tab$tch_src$TCH_AUX

END

done
