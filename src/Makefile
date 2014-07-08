# Source files
GEN_SRC = aim_constants.cpp \
		Aim.cpp \
		aim_types.cpp \
		AimServer.cpp \
		RimpUtils.cpp \
		Rimp.cpp \
		VLan.cpp \
		StringUtils.cpp \
		StorageService.cpp \
		LibvirtService.cpp \
		ExecUtils.cpp \
		pugixml/pugixml.cpp \
		inih/ini.c \
		inih/INIReader.cpp \
		version.c

CPP_OBJ = $(GEN_SRC:%.cpp=%.o)
GEN_OBJ = $(CPP_OBJ:%.c=%.o)

# Force version generation in each build
VERSION := $(shell sh gen-version.sh)

# Include directories
USR_INC = /usr/include
USRLOCAL_INC = /usr/local/include
VIRT_INC = /usr/include/libvirt
CURL_INC = /usr/include/curl
PUGIXML_INC = ./pugixml
INIH_INC = ./inih

# Include and link paths
INC_PATH = -I$(USR_INC) -I$(USRLOCAL_INC) -I$(VIRT_INC) -I$(CURL_INC) -I$(PUGIXML_INC) -I$(INIH_INC) -I.
LD_PATH = -L/lib64 -L/usr/lib64 -L/usr/local/lib

# Compiler and linker flags
# https://issues.apache.org/jira/browse/THRIFT-1326
THRIFT_FLAGS = -DHAVE_NETINET_IN_H -DHAVE_INTTYPES_H
CC = g++
CFLAGS = -Wall
CXXFLAGS = -Wall $(THRIFT_FLAGS) $(INC_PATH)
LDFLAGS = $(LD_PATH) -lpthread -lvirt -lthrift -lthriftnb -levent -lcurl -luuid -lboost_filesystem -lboost_thread-mt


all: aim

aim: $(GEN_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o aim
	$(RM) pugixml/*.o
	$(RM) inih/*.o

.PHONY: all clean

