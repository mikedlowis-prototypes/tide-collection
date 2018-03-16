INCS = -Iinc -I/usr/include/freetype2/
LIBS = -lX11 -lXft

BINS = bin/pick bin/xcpd

all: $(BINS)

clean:
	$(RM) $(BINS)

bin/%: src/%.c
	cc $(INCS) -o $@ $^ $(LIBS)

include deps.mk
