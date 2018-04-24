MAKEFLAGS = -j4

INCS = \
    -Iinc \
    -I/usr/X11/include \
    -I/usr/include/freetype2 \
    -I/usr/X11/include/freetype2

LIBS = -L/usr/X11/lib -lX11 -lXft

BINS = bin/pick bin/xcpd bin/tfetch bin/tctl bin/eol bin/plumb

all: $(BINS)

clean:
	$(RM) $(BINS)

bin/%: src/%.c
	cc $(INCS) -o $@ $< $(LIBS)

include deps.mk
