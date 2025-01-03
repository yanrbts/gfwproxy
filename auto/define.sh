# Copyright (C) Yan Ruibing

cat << END >> $TCH_AUTO_CONFIG_H

#ifndef $have
#define $have  $value
#endif

END