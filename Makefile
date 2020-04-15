BIN := bin
CORE := src/core
BUILD := build
PROGDIR := src/programs
MAINSRC := $(PROGDIR)/main.cpp
MAINOBJ := $(BUILD)/main.o

TARGET := $(BIN)/sword

SRCS := $(shell find $(CORE) -name "*.cpp")
SRCS := $(shell cd $(CORE) && find -name "*.cpp" | cut -c3-)
SRCS := $(SRCS) main.cpp
OBJS := $(SRCS:%=$(BUILD)/%)
OBJS := $(OBJS:.cpp=.o) 
OBJS := $(OBJS)

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
#DEPFLAGS = -MT $@ -MMD -MP -MF -

CC = g++
INC_DIRS := -I$(CORE) -I/usr/include -I./include/thirdparty

WFLAGS = -Wall -W -Wno-parentheses -Wno-unused-variable -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wno-unused-parameter -Wno-unused-local-typedefs
CPPFLAGS = $(DEPFLAGS) -g -std=c++2a $(WFLAGS) $(INC_DIRS) -fPIC 
LDFLAGS = -lpthread -lxcb -lvulkan -lX11 -lreadline -ldl
LIB = ./lib
LDIRS = -L$(LIB) #-L$(LIB)/loader 
LINK = $(LDIRS) $(LDFLAGS)
SPV = ./build/shadersstar
THIRD = ./src/thirdparty

$(TARGET) : $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS) ; ctags -R . ; cd $(CORE) ; ctags -R . 

$(BUILD)/%.o : $(CORE)/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o : $(PROGDIR)/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(DEPDIR) : ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

.PHONY: clean shaders test

clean:
	find $(BUILD) -type f -delete 

shaders: 
	python3 tools/compileShaders.py

test:
	@echo '$(DEPFILES)'

maim:
	@echo '$(PROGDIR)/main.cpp $(DEPDIR)/main.d | $(DEPDIR)'
include $(wildcard $(DEPFILES))
