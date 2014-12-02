INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

CC=g++
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: filesystem transaction aggregate

## Variables and rules for FILE
SOURCES_FILE:= Node.cpp File.cpp Directory.cpp ../filesystem.cpp\
	../common/helper.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/filesystem/,$(SOURCES_FILE))
HEADERS_FILE:=Node.h File.h Directory.h ../common/helper.h
FHEADERS_FILE:=$(addprefix $(INCDIR)/filesystem/,$(HEADERS_FILE))
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/filesystem/,$(OBJECTS_FILE))
EXECFILE:=$(BINDIR)/filesystem
LDFLAGS_FILE=-lboost_system -lboost_filesystem -lcrypto

filesystem: $(EXECFILE) $(FHEADERS_FILE)
$(EXECFILE): $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_FILE) -g -o $@ $(LDFLAGS_FILE)

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


## Variables and rules for AGGREGATE
SOURCES_AGGREGATE:=transaction/HttpTransaction.cpp transaction/Transaction.cpp\
	transaction/BasicTransaction.cpp\ transaction/RemoteData.cpp\
	transaction/RemoteDataHttp.cpp transaction/Range.cpp \
	transaction/transaction.cpp\
	filesystem/Node.cpp filesystem/File.cpp filesystem/Directory.cpp\
	common/helper.cpp\
	aggregate/Chunk.cpp
FSOURCES_AGGREGATE:=$(addprefix $(SRCDIR)/,$(SOURCES_AGGREGATE))
HEADERS_AGGREGATE:=transaction/BasicTransaction.h transaction/Transaction.h\
	transaction/HttpTransaction.h transaction/RemoteData.h\
	transaction/RemoteDataHttp.h transaction/Range.h\
	filesystem/Node.h filesystem/File.h filesystem/Directory.h\
	common/helper.h\
	aggregate/Chunk.h

FHEADERS_AGGREGATE:=$(addprefix $(INCDIR)/,$(HEADERS_AGGREGATE))
OBJECTS_AGGREGATE:=$(SOURCES_AGGREGATE:.cpp=.o)
FOBJECTS_AGGREGATE:=$(addprefix $(OBJDIR)/,$(OBJECTS_AGGREGATE))
EXECAGGREGATE:=$(BINDIR)/aggregate
LDFLAGS_AGGREGATE=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

aggregate: $(EXECAGGREGATE) $(FHEADERS_AGGREGATE)
$(EXECAGGREGATE): $(FOBJECTS_AGGREGATE)
	$(CC) $(FOBJECTS_AGGREGATE) -g -o $@ $(LDFLAGS_AGGREGATE)


clean:
	-rm -rf bin/*

## Common parts to both
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) -o $@ $< $(CFLAGS)

$(OBJDIR): | $(BINDIR)
	mkdir $(OBJDIR) $(OBJDIR)/filesystem $(OBJDIR)/transaction\
		$(OBJDIR)/common $(OBJDIR)/aggregate

$(BINDIR):
	mkdir $(BINDIR)


