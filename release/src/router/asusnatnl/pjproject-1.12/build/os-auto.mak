# build/os-auto.mak.  Generated from os-auto.mak.in by configure.

export OS_CFLAGS   := $(CC_DEF)PJ_AUTOCONF=1 -I/home/plp/k2p/asuswrt/release/src-rt-9.x/src/router/openssl/include  -g -O2 -fPIC -DROUTER=1  -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1

export OS_CXXFLAGS := $(CC_DEF)PJ_AUTOCONF=1 -I/home/plp/k2p/asuswrt/release/src-rt-9.x/src/router/openssl/include  -g -O2 -fPIC -DROUTER=1  

export OS_LDFLAGS  := -L/home/plp/k2p/asuswrt/release/src-rt-9.x/src/router/openssl     -lm -lnsl -lrt -lpthread  -lssl -lcrypto    -lstdc++ -lcrypto -lssl

export OS_SOURCES  := 


