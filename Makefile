CC       =	c++
CFLAGS   =	-O2 -Wall -lpthread `fltk-config --use-images --cxxflags --ldflags` \
			`pkg-config --cflags --libs libvncclient libvncserver libssh2`
DEBUGFLGS=	-g -O0
BINDIR   = /usr/local/bin
TARGET   =	spiritvnc-fltk
SRC 	 =	`ls src/*.cxx`
PKGCONF  =	`which pkg-config`

spiritvnc-fltk:
	@if [ -z ${PKGCONF} ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

debug:
	@if [ -z ${PKGCONF} ]; then \
		echo " " ; \
		echo "#### error: 'pkg-config' not found ####" ; \
		exit 1 ; \
	fi

	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(DEBUGFLGS)

.PHONY: clean
clean::
	rm -f $(TARGET)

install:
	install -c -s -o root -m 555 $(TARGET) $(BINDIR)

uninstall:
	@if [ -f ${BINDIR}"/"${TARGET} ] ; then \
		rm -fv ${BINDIR}"/"${TARGET} ; \
	fi
