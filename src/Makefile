CPPFLAGS	= $(shell pkg-config --cflags gtkmm-3.0)
LIBS		= $(shell pkg-config --libs gtkmm-3.0)

SRCS		= $(wildcard *.cc)
OBJS		= $(patsubst %.cc, %.o, $(SRCS))

gnomoku: $(OBJS)
	$(CXX) $^ $(LIBS) -o $@