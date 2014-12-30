#
#	lib605 makefile
#
VERSION =  $(shell git rev-parse --short HEAD)

OUTPUT = lib605.so

SRCDIR = ./src
CFLAGS = -I$(SRCDIR)/include -Wall -std=c++11 -D 'VERSION="$(VERSION)"'

LDFLAGS = -fPIC -shared


all: $(OUTPUT)

default: $(OUTPUT)

$(OUTPUT):
	$(CXX) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/lib605.cpp -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)

