CPPFLAGS	= $(shell pkg-config --cflags gtkmm-3.0)
LIBS		= $(shell pkg-config --libs gtkmm-3.0)

SRCS		= $(wildcard *.cc)
OBJS		= $(patsubst %.cc, %.o, $(SRCS))

gnomoku: $(OBJS)
	$(CXX) $^ $(LIBS) -o $@

clean:
	rm -f gnomoku $(OBJS)

pic_%.h: ../pic/%.png
	gdk-pixbuf-csource --raw --name=$(shell basename $@ .h) $< > $@

mainwin.o: pic_gnomoku.h

point.o: pic_px0.h pic_px1.h pic_px2.h

.phony: clean
