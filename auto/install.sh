# Copyright (C) Yan Ruibing

# create Makefile

cat << END >> Makefile

build:
	\$(MAKE) -f $TCH_MAKEFILE

install:
	\$(MAKE) -f $TCH_MAKEFILE install

modules:
	\$(MAKE) -f $TCH_MAKEFILE modules

END
