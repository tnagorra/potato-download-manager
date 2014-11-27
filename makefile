INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

CC=g++
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: filesystem transaction

## Variables and rules for FILE
SOURCES_FILE:=filesystem.cpp filesystem/Node.cpp filesystem/File.cpp filesystem/Directory.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/,$(SOURCES_FILE))
HEADERS_FILE:=common/ex.h filesystem/ex.h filesystem/Node.h filesystem/File.h filesystem/Directory.h
FHEADERS_FILE:=$(addprefix $(INCDIR)/,$(HEADERS_FILE))
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/,$(OBJECTS_FILE))
EXECFILE:=$(BINDIR)/filesystem
LDFLAGS_FILE=-lboost_system -lboost_filesystem

filesystem: $(EXECFILE) $(FHEADERS_FILE)

$(EXECFILE): $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_FILE) -o $@ $(LDFLAGS_FILE)

## Variables and rules for DOWN
SOURCES_DOWN:=HttpTransaction.cpp Transaction.cpp BasicTransaction.cpp\
    RemoteData.cpp RemoteDataHttp.cpp Range.cpp ../transaction.cpp
FSOURCES_DOWN:=$(addprefix $(SRCDIR)/transaction/,$(SOURCES_DOWN))
HEADERS_DOWN:=BasicTransaction.h Transaction.h HttpTransaction.h\
    RemoteData.h RemoteDataHttp.h Range.h
FHEADERS_DOWN:=$(addprefix $(INCDIR)/transaction/,$(HEADERS_DOWN))
OBJECTS_DOWN:=$(SOURCES_DOWN:.cpp=.o)
FOBJECTS_DOWN:=$(addprefix $(OBJDIR)/transaction/,$(OBJECTS_DOWN))
EXECDOWN:=$(BINDIR)/transaction
LDFLAGS_DOWN=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

transaction: $(EXECDOWN) $(FHEADERS_DOWN)
$(EXECDOWN): $(FOBJECTS_DOWN)
	$(CC) $(FOBJECTS_DOWN) -g -o $@ $(LDFLAGS_DOWN)

clean:
	-rm -rf bin/*

## Common parts to both
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) -o $@ $< $(CFLAGS)

$(OBJDIR): | $(BINDIR)
	mkdir $(OBJDIR) $(OBJDIR)/filesystem $(OBJDIR)/transaction

$(BINDIR):
	mkdir $(BINDIR)

