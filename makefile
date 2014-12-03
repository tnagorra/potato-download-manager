INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

# CC=g++
CC=clang++-3.5
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: filesystem transaction

## Variables and rules for FILE
SOURCES_FILE:= Node.cpp File.cpp Directory.cpp ../common/helper.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/filesystem/,$(SOURCES_FILE))
HEADERS_FILE:=Node.h File.h Directory.h ../common/helper.h
FHEADERS_FILE:=$(addprefix $(INCDIR)/filesystem/,$(HEADERS_FILE))
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/filesystem/,$(OBJECTS_FILE))
EXECFILE:=$(BINDIR)/filesystem
LDFLAGS_FILE=-lboost_system -lboost_filesystem -lcrypto

MAINO_FILE:=$(OBJDIR)/filesystem.o

filesystem: $(FHEADERS_FILE) $(EXECFILE)
$(EXECFILE): $(FOBJECTS_FILE) $(MAINO_FILE)
	$(CC) $(FOBJECTS_FILE) $(MAINO_FILE) -g -o $@ $(LDFLAGS_FILE)

## Variables and rules for DOWN
SOURCES_DOWN:=HttpTransaction.cpp Transaction.cpp BasicTransaction.cpp\
    RemoteData.cpp RemoteDataHttp.cpp Range.cpp
MAINO_DOWN:=$(OBJDIR)/transaction.o
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
$(EXECDOWN): $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE) \
	-g -o $@ $(LDFLAGS_DOWN) $(LDFLAGS_FILE)


## Variables and rules for AGGREGATE

SOURCES_AGGREGATE:= aggregate/Chunk.cpp aggregate/Aggregate.cpp
HEADERS_AGGREGATE:= aggregate/Chunk.h aggregate/Aggregate.h
MAINO_AGGREGATE:=$(OBJDIR)/transaction.o
OBJECTS_AGGREGATE:=$(SOURCES_AGGREGATE:.cpp=.o)
EXECAGGREGATE:=$(BINDIR)/aggregate
FSOURCES_AGGREGATE:=$(addprefix $(SRCDIR)/,$(SOURCES_AGGREGATE))
FSOURCES_AGGREGATE:=$(addprefix $(SRCDIR)/,$(SOURCES_AGGREGATE))
FHEADERS_AGGREGATE:=$(addprefix $(INCDIR)/,$(HEADERS_AGGREGATE))
FOBJECTS_AGGREGATE:=$(addprefix $(OBJDIR)/,$(OBJECTS_AGGREGATE))
LDFLAGS_AGGREGATE=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

aggregate: $(EXECAGGREGATE) $(FHEADERS_AGGREGATE)
$(EXECAGGREGATE): $(FOBJECTS_AGGREGATE) $(MAINO_AGGREGATE) $(FOBJECTS_FILE) $(FOBJECTS_DOWN)
	$(CC) $(FOBJECTS_AGGREGATE) $(MAINO_AGGREGATE) $(FOBJECTS_FILE) $(FOBJECTS_DOWN) \
	-g -o $@ $(LDFLAGS_AGGREGATE) $(LDFLAGS_FILE) $(LDFLAGS_DOWN)

### Common parts to both
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) -o $@ $< $(CFLAGS)

$(OBJDIR): | $(BINDIR)
	mkdir $(OBJDIR) $(OBJDIR)/filesystem $(OBJDIR)/transaction\
		$(OBJDIR)/common $(OBJDIR)/aggregate

$(BINDIR):
	mkdir $(BINDIR)

clean:
	-rm -rf bin/*
