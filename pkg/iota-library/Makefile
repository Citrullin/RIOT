PKG_NAME=iota-library
PKG_URL=https://github.com/embedded-iota/iota-c-library
PKG_VERSION=ff286cb284909d8fa5450e481462db91324b5836
PKG_LICENSE=MIT

.PHONY: all

RIOT_CFLAGS = $(INCLUDES) -std=c99

TOOLCHAIN_FILE=$(PKG_BUILDDIR)/xcompile-toolchain.cmake

all: $(PKG_BUILDDIR)/src/Makefile
	$(MAKE) -C $(PKG_BUILDDIR) && \
	cp $(PKG_BUILDDIR)/build/libiota.a $(BINDIR)/iota-library.a

$(PKG_BUILDDIR)/src/Makefile: $(TOOLCHAIN_FILE)
	cd $(PKG_BUILDDIR) && \
	mkdir -p build && \
	make -e CFLAGS+="$(RIOT_CFLAGS)" lib

$(TOOLCHAIN_FILE): git-download
	$(RIOTTOOLS)/cmake/generate-xcompile-toolchain.sh > $(TOOLCHAIN_FILE)

include $(RIOTBASE)/pkg/pkg.mk
