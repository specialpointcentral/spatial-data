CXX=g++
LIBS=zorder_knn/zorder_knn/include
INCLUDED=-I$(LIBS) -I.
CPPFLAGS=-O2
LDFLAGS=-lm

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))
DEPS=$(OBJS:.o=.d)

all: spatial

spatial: $(OBJS)
	$(CXX) $(LDFLAGS) -o spatial $(OBJS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDED) -MM -MF $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CPPFLAGS) $(INCLUDED) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(DEPS)

distclean: clean
	$(RM) *~ spatial

-include $(DEPS)

