#
#	lib605 makefile
#
VERSION =  $(shell git rev-parse --short HEAD)

OUTPUT = lib605.so

SRCDIR = ./src
CFLAGS = -I$(SRCDIR)/include -Wall -std=c++11 -D 'VERSION="$(VERSION)"'

LDFLAGS = -fPIC -shared


all: $(OUTPUT) docs

default: $(OUTPUT)

$(OUTPUT):
	$(CXX) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/lib605.cpp -o $(OUTPUT)

docs:
	gzip -fq9k lib605.3

clean:
	rm -f $(OUTPUT) lib605.3.gz

