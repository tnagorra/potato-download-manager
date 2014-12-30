INCDIR:=include
SRCDIR:=src
OBJDIR:=bin/obj
BINDIR:=bin

#CC=g++
CC=clang
CFLAGS=-c -I$(INCDIR)/ --std=c++11
#CFLAGS=-c -Wall -I$(INCDIR)/ --std=c++11

all: filesystem transaction aggregator options

## Variables and rules for FILE
SOURCES_FILE:= Node.cpp File.cpp Directory.cpp
HEADERS_FILE:=Node.h File.h Directory.h
OBJECTS_FILE:=$(SOURCES_FILE:.cpp=.o)
FSOURCES_FILE:=$(addprefix $(SRCDIR)/filesystem/,$(SOURCES_FILE))
FHEADERS_FILE:=$(addprefix $(INCDIR)/filesystem/,$(HEADERS_FILE))
FOBJECTS_FILE:=$(addprefix $(OBJDIR)/filesystem/,$(OBJECTS_FILE))
LDFLAGS_FILE=-lboost_system -lboost_filesystem -lcrypto

## Variables and rules for common
SOURCES_COMM:= helper.cpp
HEADERS_COMM:=helper.h ex.h
OBJECTS_COMM:=$(SOURCES_COMM:.cpp=.o)
FSOURCES_FILE:=$(addprefix $(SRCDIR)/common/,$(SOURCES_COMM))
FHEADERS_COMM:=$(addprefix $(INCDIR)/common/,$(HEADERS_COMM))
FOBJECTS_COMM:=$(addprefix $(OBJDIR)/common/,$(OBJECTS_COMM))
LDFLAGS_COMM=-lboost_system -lboost_filesystem -lcrypto

## Variables and rules for DOWN
SOURCES_DOWN:=HttpTransaction.cpp Transaction.cpp BasicTransaction.cpp\
    RemoteData.cpp RemoteDataHttp.cpp Range.cpp
HEADERS_DOWN:=BasicTransaction.h Transaction.h HttpTransaction.h\
    RemoteData.h RemoteDataHttp.h Range.h
OBJECTS_DOWN:=$(SOURCES_DOWN:.cpp=.o)
FSOURCES_DOWN:=$(addprefix $(SRCDIR)/transaction/,$(SOURCES_DOWN))
FHEADERS_DOWN:=$(addprefix $(INCDIR)/transaction/,$(HEADERS_DOWN))
FOBJECTS_DOWN:=$(addprefix $(OBJDIR)/transaction/,$(OBJECTS_DOWN))
LDFLAGS_DOWN=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

## Variables and rules for AGGREGATE
SOURCES_AGGREGATE:= Chunk.cpp Aggregate.cpp
HEADERS_AGGREGATE:= Chunk.h Aggregate.h
OBJECTS_AGGREGATE:=$(SOURCES_AGGREGATE:.cpp=.o)
FSOURCES_AGGREGATE:=$(addprefix $(SRCDIR)/aggregator/,$(SOURCES_AGGREGATE))
FHEADERS_AGGREGATE:=$(addprefix $(INCDIR)/aggregator/,$(HEADERS_AGGREGATE))
FOBJECTS_AGGREGATE:=$(addprefix $(OBJDIR)/aggregator/,$(OBJECTS_AGGREGATE))
LDFLAGS_AGGREGATE=-lboost_system -lboost_filesystem -lboost_thread\
	-lssl -lcrypto -pthread

## Variables and rules for OPTION
SOURCES_OPTION:= CommonOptions.cpp LocalOptions.cpp GlobalOptions.cpp
HEADERS_OPTION:= CommonOptions.h LocalOptions.h GlobalOptions.h
OBJECTS_OPTION:=$(SOURCES_OPTION:.cpp=.o)
FSOURCES_OPTION:=$(addprefix $(SRCDIR)/options/,$(SOURCES_OPTION))
FHEADERS_OPTION:=$(addprefix $(INCDIR)/options/,$(HEADERS_OPTION))
FOBJECTS_OPTION:=$(addprefix $(OBJDIR)/options/,$(OBJECTS_OPTION))
LDFLAGS_OPTION=-lboost_system -lboost_program_options

MAINO_FILE:=$(OBJDIR)/filesystem.o
MAINO_DOWN:=$(OBJDIR)/transaction.o
MAINO_AGGREGATE:=$(OBJDIR)/aggregator.o
MAINO_OPTION:=$(OBJDIR)/options.o

EXEC_FILE:=$(BINDIR)/filesystem
EXEC_DOWN:=$(BINDIR)/transaction
EXEC_AGGREGATE:=$(BINDIR)/aggregator
EXEC_OPTION:=$(BINDIR)/options

filesystem: $(FHEADERS_FILE) $(FHEADERS_COMM) $(EXEC_FILE)
$(EXEC_FILE): $(FOBJECTS_FILE) $(FOBJECTS_COMM) $(MAINO_FILE)
	$(CC) $(FOBJECTS_FILE) $(FOBJECTS_COMM) $(MAINO_FILE) -g -o $@ $(LDFLAGS_FILE)

transaction: $(EXEC_DOWN) $(FHEADERS_DOWN)
$(EXEC_DOWN): $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE)
	$(CC) $(FOBJECTS_DOWN) $(MAINO_DOWN) $(FOBJECTS_FILE) \
	-g -o $@ $(LDFLAGS_DOWN) $(LDFLAGS_FILE)

options: $(EXEC_OPTION) $(FHEADERS_OPTION) $(FHEADERS_COMM)
$(EXEC_OPTION): $(FOBJECTS_OPTION) $(MAINO_OPTION) $(FOBJECTS_FILE) $(FOBJECTS_COMM)
	$(CC) $(FOBJECTS_OPTION) $(MAINO_OPTION) $(FOBJECTS_FILE) $(FOBJECTS_COMM) \
	-g -o $@ $(LDFLAGS_OPTION) $(LDFLAGS_FILE)

aggregator: $(EXEC_AGGREGATE) $(FHEADERS_AGGREGATE) $(FHEADERS_DOWN)\
    $(FHEADERS_FILE) $(FHEADERS_OPTION) $(FHEADERS_COMM)
$(EXEC_AGGREGATE): $(FOBJECTS_AGGREGATE) $(MAINO_AGGREGATE) $(FOBJECTS_FILE)\
	$(FOBJECTS_DOWN) $(FOBJECTS_OPTION) $(FOBJECTS_COMM)
	$(CC) $(FOBJECTS_AGGREGATE) $(MAINO_AGGREGATE) $(FOBJECTS_FILE)\
		$(FOBJECTS_DOWN) $(FOBJECTS_OPTION) $(FOBJECTS_COMM)\
		-g -o $@ $(LDFLAGS_AGGREGATE) $(LDFLAGS_FILE) $(LDFLAGS_DOWN) $(LDFLAGS_OPTION)


### Common parts to both
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) -o $@ $< $(CFLAGS)

$(OBJDIR): | $(BINDIR)
	mkdir $(OBJDIR) $(OBJDIR)/filesystem $(OBJDIR)/transaction\
		$(OBJDIR)/common $(OBJDIR)/aggregator $(OBJDIR)/options

$(BINDIR):
	mkdir $(BINDIR)

clean:
	-rm -rf bin/*
