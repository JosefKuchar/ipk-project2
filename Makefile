CPPFLAGS = -std=c++20 -O2

# Get all .c files
SRCS = $(wildcard *.cc)
# Get corresponding .o files
OBJS := $(SRCS:%.cc=%.o)
# Get corresponding .d files
DEPS := $(SRCS:%.cc=%.d)

# These will run every time (not just when the files are newer)
.PHONY: run_tcp run_udp clean zip test

# Main target
ipkcpd: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Dependecies
%.o: %.cc %.d
	$(CC) -MT $@ -MMD -MP -MF $*.d $(CFLAGS) $(CPPFLAGS) -c $(OUTPUT_OPTION) $<
$(DEPS):
include $(wildcard $(DEPS))

clean:
	rm -f *.o *.d ipkcpd xkucha28.zip

run_tcp: ipkcpd
	./ipkcpd -h 127.0.0.1 -p 1234 -m tcp

run_udp: ipkcpd
	./ipkcpd -h 127.0.0.1 -p 1235 -m udp

zip: clean
	zip -r xkucha28.zip *
