INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

CC=g++
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: filesystem transaction

## Variables and rules for FILE
SOURCES_FILE:= Node.cpp File.cpp Directory.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/filesystem/,$(SOURCES_FILE))
HEADERS_FILE:=Node.h File.h Directory.h
FHEADERS_FILE:=$(addprefix $(INCDIR)/filesystem/,$(HEADERS_FILE))
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/filesystem/,$(OBJECTS_FILE))
EXECFILE:=$(BINDIR)/filesystem
LDFLAGS_FILE=-lboost_system -lboost_filesystem -lcrypto

## Variables and rules for common
SOURCES_COMM:= helper.cpp
FSOURCES_FILE:=$(addprefix $(SRCDIR)/common/,$(SOURCES_COMM))
HEADERS_COMM:=helper.h ex.h
FHEADERS_COMM:=$(addprefix $(INCDIR)/common/,$(HEADERS_COMM))
OBJECTS_COMM:=$(SOURCES_COMM:.cpp=.o)
FOBJECTS_COMM:=$(addprefix $(OBJDIR)/common/,$(OBJECTS_COMM))
LDFLAGS_COMM=-lboost_system -lboost_filesystem -lcrypto

## Variables and rules for DOWN
SOURCES_DOWN:=HttpTransaction.cpp Transaction.cpp BasicTransaction.cpp\
    RemoteData.cpp RemoteDataHttp.cpp Range.cpp
FSOURCES_DOWN:=$(addprefix $(SRCDIR)/transaction/,$(SOURCES_DOWN))
HEADERS_DOWN:=BasicTransaction.h Transaction.h HttpTransaction.h\
    RemoteData.h RemoteDataHttp.h Range.h
FHEADERS_DOWN:=$(addprefix $(INCDIR)/transaction/,$(HEADERS_DOWN))
OBJECTS_DOWN:=$(SOURCES_DOWN:.cpp=.o)
FOBJECTS_DOWN:=$(addprefix $(OBJDIR)/transaction/,$(OBJECTS_DOWN))
EXEC_DOWN:=$(BINDIR)/transaction
LDFLAGS_DOWN=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

## Variables and rules for AGGREGATE
SOURCES_AGGREGATE:= Chunk.cpp
FSOURCES_AGGREGATE:=$(addprefix $(SRCDIR)/aggregate,$(SOURCES_AGGREGATE))
HEADERS_AGGREGATE:= Chunk.h
FHEADERS_AGGREGATE:=$(addprefix $(INCDIR)/aggregate,$(HEADERS_AGGREGATE))
OBJECTS_AGGREGATE:=$(SOURCES_AGGREGATE:.cpp=.o)
FOBJECTS_AGGREGATE:=$(addprefix $(OBJDIR)/aggregate,$(OBJECTS_AGGREGATE))
EXEC_AGGREGATE:=$(BINDIR)/aggregate
LDFLAGS_AGGREGATE=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

MAINO_FILE:=$(OBJDIR)/filesystem.o
MAINO_DOWN:=$(OBJDIR)/transaction.o
MAINO_AGGREGATE:=$(OBJDIR)/aggregate.o

filesystem: $(FHEADERS_FILE) $(FHEADERS_COMM) $(EXEC_FILE) 
$(EXEC_FILE): $(FOBJECTS_FILE) $(FOBJECTS_COMM) $(MAINO_FILE)
	$(CC) $(FOBJECTS_FILE) $(FOBJECTS_COMM) $(MAINO_FILE) -g -o $@ $(LDFLAGS_FILE)

transaction: $(EXEC_DOWN) $(FHEADERS_DOWN)
$(EXEC_DOWN): $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE) \
	-g -o $@ $(LDFLAGS_DOWN) $(LDFLAGS_FILE)

aggregate: $(EXECAGGREGATE) $(FHEADERS_AGGREGATE)
$(EXEC_AGGREGATE): $(FOBJECTS_AGGREGATE) $(MAINO_AGGREGATE) $(FOBJECTS_FILE) $(FOBJECTS_DOWN)
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
