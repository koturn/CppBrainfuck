ifeq ($(DEBUG),true)
    OPT_CFLAGS   := -O0 -g3 -ftrapv -fstack-protector-all -D_FORTIFY_SOURCE=2
    OPT_LDLIBS   := -lssp
ifneq ($(shell echo $$OSTYPE),cygwin)
    OPT_CFLAGS   += -fsanitize=address -fno-omit-frame-pointer
    OPT_LDLIBS   += -fsanitize=address
endif
    OPT_CXXFLAGS := $(OPT_CFLAGS) -D_GLIBCXX_DEBUG
else
ifeq ($(OPT),true)
    OPT_CFLAGS   := -flto -Ofast -march=native -DNDEBUG
    OPT_CXXFLAGS := $(OPT_CFLAGS)
    OPT_LDFLAGS  := -flto -s
else
ifeq ($(LTO),true)
    OPT_CFLAGS   := -flto -DNDEBUG
    OPT_CXXFLAGS := $(OPT_CFLAGS)
    OPT_LDFLAGS  := -flto
else
    OPT_CFLAGS   := -O3 -DNDEBUG
    OPT_CXXFLAGS := $(OPT_CFLAGS)
    OPT_LDFLAGS  := -s
endif
endif
endif

WARNING_COMMON_FLAGS := \
    -Wall \
    -Wextra \
    -Wabi \
    -Wcast-align \
    -Wcast-qual \
    -Wconversion \
    -Wdisabled-optimization \
    -Wdouble-promotion \
    -Wfloat-equal \
    -Wformat=2 \
    -Winit-self \
    -Wlogical-op \
    -Wmissing-declarations \
    -Wno-return-local-addr \
    -Wpointer-arith \
    -Wredundant-decls \
    -Wstrict-aliasing=2 \
    -Wsuggest-attribute=const \
    -Wsuggest-attribute=format \
    -Wsuggest-attribute=noreturn \
    -Wsuggest-attribute=pure \
    -Wsuggest-final-methods \
    -Wsuggest-final-types \
    -Wswitch-enum \
    -Wundef \
    -Wunsafe-loop-optimizations \
    -Wunreachable-code \
    -Wvector-operation-performance \
    -Wwrite-strings \
    -pedantic

WARNING_CFLAGS := \
    $(WARNING_COMMON_FLAGS) \
    -Wc++-compat \
    -Wbad-function-cast \
    -Wjump-misses-init \
    -Wmissing-prototypes \
    -Wunsuffixed-float-constants

WARNING_CXXFLAGS := \
    $(WARNING_COMMON_FLAGS) \
    -Wc++11-compat \
    -Wc++14-compat \
    -Weffc++ \
    -Woverloaded-virtual \
    -Wsign-promo \
    -Wstrict-null-sentinel \
    -Wsuggest-override \
    -Wuseless-cast \
    -Wzero-as-null-pointer-constant

CC         := gcc $(if $(STDC),$(addprefix -std=,$(STDC)),-std=gnu11)
CXX        := g++ $(if $(STDCXX),$(addprefix -std=,$(STDCXX)),-std=gnu++14)
MKDIR      := mkdir -p
CP         := cp
RM         := rm -f
CTAGS      := ctags
# MACROS   := MACRO
# INCDIRS  := ./include
CPPFLAGS   := $(addprefix -D,$(MACROS)) $(addprefix -I,$(INCDIRS))
CFLAGS     := -pipe $(WARNING_CFLAGS) $(OPT_CFLAGS)
CXXFLAGS   := -pipe $(WARNING_CXXFLAGS) $(OPT_CXXFLAGS)
LDFLAGS    := -pipe $(OPT_LDFLAGS)
LDLIBS     := $(OPT_LDLIBS)
CTAGSFLAGS := -R --languages=c,c++
TARGET     := brainfuck
SRCS       := $(wildcard *.cpp)
OBJS       := $(foreach PAT,%.cpp %.cxx %.cc,$(patsubst $(PAT),%.o,$(filter $(PAT),$(SRCS))))
DEPENDS    := depends.mk

ifeq ($(OS),Windows_NT)
    TARGET := $(addsuffix .exe,$(TARGET))
else
    TARGET := $(addsuffix .out,$(TARGET))
endif
INSTALLED_TARGET := $(if $(PREFIX),$(PREFIX),/usr/local)/bin/$(TARGET)

%.exe:
	$(CXX) $(LDFLAGS) $(filter %.c %.cpp %.cxx %.cc %.o,$^) $(LDLIBS) -o $@
%.out:
	$(CXX) $(LDFLAGS) $(filter %.c %.cpp %.cxx %.cc %.o,$^) $(LDLIBS) -o $@


.PHONY: all test depends syntax ctags install uninstall clean disclean
all: $(TARGET)
$(TARGET): $(OBJS)

$(foreach SRC,$(SRCS),$(eval $(filter-out \,$(shell $(CXX) -MM $(SRC)))))

test: $(TARGET)
	$(MAKE) -C t/

depends:
	$(CXX) -MM $(SRCS) > $(DEPENDS)

syntax:
	$(CXX) $(SRCS) -fsyntax-only $(WARNING_CXXFLAGS) $(INCS) $(MACROS)

ctags:
	$(CTAGS) $(CTAGSFLAGS)

install: $(INSTALLED_TARGET)
$(INSTALLED_TARGET): $(TARGET)
	@[ ! -d $(@D) ] && $(MKDIR) $(@D) || :
	$(CP) $< $@

uninstall:
	$(RM) $(INSTALLED_TARGET)

clean:
	$(RM) $(OBJS)

distclean:
	$(RM) $(TARGET) $(OBJS)