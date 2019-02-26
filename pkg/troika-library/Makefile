PKG_NAME=troika-library
PKG_URL=https://github.com/Citrullin/troika.git
PKG_VERSION=d8549236cebda0d80d8f566435cae0256a61afd6
PKG_LICENSE=MIT

.PHONY: all

RIOT_CFLAGS = $(INCLUDES) -std=c99

TOOLCHAIN_FILE=$(PKG_BUILDDIR)/xcompile-toolchain.cmake

all: $(PKG_BUILDDIR)/src/Makefile
	$(MAKE) -C $(PKG_BUILDDIR) && \
	cp $(PKG_BUILDDIR)/build/libtroika-library.a $(BINDIR)/troika-library.a

$(PKG_BUILDDIR)/src/Makefile: $(TOOLCHAIN_FILE)
	cd $(PKG_BUILDDIR) && \
	mkdir -p build && \
	make -e CFLAGS+="$(RIOT_CFLAGS)" lib

$(TOOLCHAIN_FILE): git-download
	$(RIOTTOOLS)/cmake/generate-xcompile-toolchain.sh > $(TOOLCHAIN_FILE)

include $(RIOTBASE)/pkg/pkg.mk
