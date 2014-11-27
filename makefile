INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

CC=g++
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: file

## Variables and rules for FILE
SOURCES_FILE:=fileTest.cpp filesystem/Node.cpp filesystem/File.cpp filesystem/Directory.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/,$(SOURCES_FILE))
HEADERS_FILE:=common/ex.h filesystem/ex.h filesystem/Node.h filesystem/File.h filesystem/Directory.h
FHEADERS_FILE:=$(addprefix $(INCDIR)/,$(HEADERS_FILE))
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/,$(OBJECTS_FILE))
EXECFILE:=$(BINDIR)/fileTest
LDFLAGS_FILE=-lboost_system -lboost_filesystem

file: $(EXECFILE) $(FHEADERS_FILE)

$(EXECFILE): $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_FILE) -o $@ $(LDFLAGS_FILE)

clean:
	-rm -rf bin/*

## Common parts to both
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) -o $@ $< $(CFLAGS)

$(OBJDIR): | $(BINDIR)
	mkdir $(OBJDIR) $(OBJDIR)/filesystem

$(BINDIR):
	mkdir $(BINDIR)

