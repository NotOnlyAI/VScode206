
# Find where we're running from, so we can store generated files here.
ifeq ($(origin MAKEFILE_DIR), undefined)
	MAKEFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif

CXX = aarch64-rockchip-linux-gnu-g++
CC = aarch64-rockchip-linux-gnu-g++
AR = aarch64-rockchip-linux-gnu-ar


HDRDIRS	=./src ./include
INCLUDES=$(addprefix -I,$(HDRDIRS))


 LIBDIRS	=./lib/aarch64
 LDFLAGS	=  $(addprefix -L,$(LIBDIRS)) -Wl,-z,relro,-z,noexecstack,--disable-new-dtags,-rpath,lib/:/usr/lib/:/usr/app/lib:./lib:./lib/aarch64
# LDFLAGS += -lrt -lm -ldl -lpthread -lstdc++
 LDFLAGS += -lrknnrt -lrga
# LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_dnn -lopencv_calib3d -lopencv_features2d -lopencv_highgui
# LDFLAGS += -lopencv_plot  -lzlib 



CFLAGS =-g -Wall -O0
CXXFLAGS=$(CFLAGS) --std=c++11
# ARFLAGS := -r

# SRCDIRS	=./src ./src/deepsort
# SRCEXTS	=.c .cpp
# SOURCE	=$(foreach d, $(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))

MYFUNC_SRCS := \
$(wildcard main.cpp)\
$(wildcard src/WavRecognize/*.cpp)\
$(wildcard src/WavRecognize/*.c)



GENDIR := $(MAKEFILE_DIR)/gen/
OBJDIR := $(GENDIR)obj/
BINDIR := $(GENDIR)bin/
LIBDIR := $(GENDIR)lib/aarch64
MYFUNC_BINARY := demo


MYFUNC_OBJS := $(addprefix $(OBJDIR), \
$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(MYFUNC_SRCS))))



$(OBJDIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


$(MYFUNC_BINARY): $(MYFUNC_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MYFUNC_SRCS) -o $(MYFUNC_BINARY) $(LIBS) $(LDFLAGS)


# object files
object:$(OBJECT)
$(BINDIRS)/%.o: $(SRCDIRS)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(OPTFLAGS) $(CFLAGS) -o $@ -c $<
$(BINDIRS)/%.o: $(SRCDIRS)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(OPTFLAGS) $(CXXFLAGS) -o $@ -c $<



show:
	@echo '[File list]:'
	@echo '	PROGRAM NAME	:$(MAKEFILE_DIR)'
	@echo '	SOURCE FILES	:$(MYFUNC_OBJS)'
	@echo '	HEADER FILES	:$(HEADER)'
	@echo '	OBJECT FILES	:$(OBJECT)'
	@echo '	DEPEND FILES	:$(DEPEND)'


# Gets rid of all generated files.
clean:
	rm -rf $./gen

# Gets rid of target files only, leaving the host alone. Also leaves the lib
# directory untouched deliberately, so we can persist multiple architectures
# across builds for iOS and Android.
cleantarget:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
