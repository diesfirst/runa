BIN := bin
CORE := src/core
BUILD := build
PROGDIR := src/programs
MAINSRC := $(PROGDIR)/main.cpp
MAINOBJ := $(BUILD)/main.o

TARGET := $(BIN)/sword

SRCS := $(shell find $(CORE) -name "*.cpp")
SRCS := $(shell cd $(CORE) && find -name "*.cpp" | cut -c3-)
SRCS := $(SRCS) 
OBJS := $(SRCS:%=$(BUILD)/%)
OBJS := $(OBJS:.cpp=.o) 
OBJS := $(OBJS)

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
#DEPFLAGS = -MT $@ -MMD -MP -MF -

CC = g++
STDFLAG = -std=c++2a
INC_DIRS := -I$(CORE) -I/usr/include -I./include/thirdparty

WFLAGS = -Wall -Wextra -W -Wno-parentheses -Wno-unused-variable -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wno-unused-parameter -Wno-unused-local-typedefs
HDKFLAGS = -D_GLIBCXX_USE_CXX11_ABI=0
CPPFLAGS = $(DEPFLAGS) -g $(HDKFLAGS) $(STDFLAG) $(WFLAGS) $(INC_DIRS) -fPIC -fconcepts
LDFLAGS = -lpthread -lxcb -lvulkan -lX11 -lreadline -ldl -lshaderc_combined -lglslc -lshaderc_util -llodepng -lm
LIB = ./lib
LDIRS = -L$(LIB) #-L$(LIB)/loader 
LINK = $(LDIRS) $(LDFLAGS)
SPV = ./build/shadersstar
THIRD = ./src/thirdparty

$(TARGET) : $(OBJS) 
	$(CC) $(OBJS) $(CPPFLAGS) $(PROGDIR)/main.cpp -o $@ $(LINK) ; ctags -R . ; cd $(CORE) ; ctags -R . 

$(BUILD)/%.o : $(CORE)/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o : $(PROGDIR)/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(DEPDIR) : ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

.PHONY: clean shaders test lib

clean:
	find $(BUILD) -type f -delete ; find $(DEPDIR) -type f -delete

shaders: 
	python3 tools/compileShaders.py

lodepng:
	$(CC) -c $(STDFLAG) -fPIC $(HDKFLAGS) $(THIRD)/lodepng.cpp -o $(THIRD)/lodepng.o ; ar rcs lib/liblodepng.a $(THIRD)/lodepng.o

shaderboy: $(OBJS)
	$(CC) $(OBJS) $(CPPFLAGS) $(PROGDIR)/shaderboy.cpp -o $(BIN)/shaderboy $(LINK)

test:
	@echo '$(DEPFILES)'

lib : $(OBJS)
	ar rcs lib/libsword.a $(OBJS)

shared : $(OBJS)
	$(CC) -shared -o lib/libsword.so $(OBJS)

maim:
	@echo '$(PROGDIR)/main.cpp $(DEPDIR)/main.d | $(DEPDIR)'



include $(wildcard $(DEPFILES))
