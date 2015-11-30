# Lix Makefile
# See ./doc/linux.txt if you want to compile yourself.

CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra
LD       = libtool --tag=CXX --mode=link $(CXX)
PKG_CONFIG ?= pkg-config

LDALLEG  = $(shell allegro-config --libs) -Wl,-rpath,./bin/lib:./lib
LDENET   = $(shell $(PKG_CONFIG) --libs libenet 2>/dev/null || echo "-L/usr/local/lib -lenet")
LDPNG    = $(shell $(PKG_CONFIG) --libs libpng zlib 2>/dev/null || echo "-lpng -lz")
LDCONFIG    = $(shell $(PKG_CONFIG) --libs libconfig++ 2>/dev/null || echo "-lconfig++")
LDPOCO    = $(shell $(PKG_CONFIG) --libs libPoco 2>/dev/null || echo "-L/usr/local/lib -lPocoNet -lPocoJSON -lPocoUtil -lPocoFoundation")
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags libpng zlib libenet libPocoJSON libPocoNet libPocoUtil libPocoFoundation libconfig++ 2>/dev/null) $(shell allegro-config --cflags)

STRIP    = strip

DEPGEN   = $(CXX) $(CPPFLAGS) -MM
RM       = rm -rf
MKDIR    = mkdir -p

SRCDIR   = src
OBJDIR   = objlin
DEPDIR   = $(OBJDIR)
BINDIR   = binlin

# verbosity switch
V=0
ifeq ($(V),0)
Q=@
endif

CLIENT_BIN  = $(BINDIR)/lix
CLIENT_CSRC = $(wildcard src/graphic/png/*.c)
CLIENT_SRCS = $(wildcard src/api/*.cpp) \
              $(wildcard src/api/button/*.cpp) \
              $(wildcard src/editor/*.cpp) \
              $(wildcard src/exposer/*.cpp) \
              $(wildcard src/gameplay/gui/*.cpp) \
              $(wildcard src/gameplay/*.cpp) \
              $(wildcard src/graphic/*.cpp) \
              $(wildcard src/lix/*.cpp) \
              $(wildcard src/level/*.cpp) \
              $(wildcard src/menu/*.cpp) \
              $(wildcard src/network/*.cpp) \
              $(wildcard src/other/*.cpp) \
              $(wildcard src/other/file/*.cpp)
CLIENT_OBJS = $(subst $(SRCDIR)/,$(OBJDIR)/,$(CLIENT_CSRC:%.c=%.o)) \
              $(subst $(SRCDIR)/,$(OBJDIR)/,$(CLIENT_SRCS:%.cpp=%.o))
CLIENT_DEPS = $(subst $(SRCDIR)/,$(DEPDIR)/,$(CLIENT_CSRC:%.c=%.d)) \
              $(subst $(SRCDIR)/,$(DEPDIR)/,$(CLIENT_SRCS:%.cpp=%.d))

SERVER_BIN  = $(BINDIR)/lixd
SERVER_SRCS = $(wildcard src/daemon/*.cpp) \
               src/network/net_t.cpp        src/network/permu.cpp \
               src/network/server.cpp       src/network/server_c.cpp
SERVER_OBJS = $(subst $(SRCDIR)/,$(OBJDIR)/,$(SERVER_SRCS:%.cpp=%.o))
SERVER_DEPS = $(subst $(SRCDIR)/,$(DEPDIR)/,$(SERVER_SRCS:%.cpp=%.d))



###############################################################################

# Replacement variables for cross-compiling Lix on Linux for Windows.
# All variable names for cross-compilation are prefixed with CRO_.
# Some non-CRO variables are used in both the Linux and the Windows target.
# CRO_WINDRES is used to compile the icon into an object file.

CRO_CXX     = i586-mingw32msvc-g++
CRO_LD      = i586-mingw32msvc-g++
CRO_WINDRES = i586-mingw32msvc-windres

# change CRO_MINGDIR to your MinGW's "i586-..." directory.
# It should sit inside /usr or /usr/local.
# Maybe you don't have to change it, because $(CRO_LD) will know it by itself.
CRO_MINGDIR  = /usr/i586-mingw32msvc

CRO_LDALLEG  = -L$(CRO_MINGDIR)/lib --subsystem,windows -mwindows -lalleg44.dll
#CRO_LDALLEG  = -L$(CRO_MINGDIR)/lib -mwindows -lalleg44.dll
CRO_LDENET   = -L$(CRO_MINGDIR)/lib -lenet -lws2_32 -lwinmm
CRO_LDPNG    = -L$(CRO_MINGDIR)/lib -lpng -lz
CRO_CPPFLAGS = -I$(CRO_MINGDIR)/include

CRO_OBJDIR   = objcross
CRO_BINDIR   = bincross

CRO_CLIENT_BIN  = $(CRO_BINDIR)/lix.exe
CRO_SERVER_BIN  = $(CRO_BINDIR)/lixserv.exe

CRO_ICON_SRC    = $(SRCDIR)/icon.rc

CRO_CLIENT_OBJS = $(subst $(SRCDIR)/,$(CRO_OBJDIR)/,$(CLIENT_CSRC:%.c=%.o)) \
                  $(subst $(SRCDIR)/,$(CRO_OBJDIR)/,$(CLIENT_SRCS:%.cpp=%.o))
CRO_ICON_OBJ    = $(CRO_OBJDIR)/icon.res
CRO_SERVER_OBJS = $(subst $(SRCDIR)/,$(CRO_OBJDIR)/,$(SERVER_SRCS:%.cpp=%.o))




###############################################################################

# Replacement variables for compiling Lix on for Windows.
# All variable names for cross-compilation are prefixed with WIN_.
# Some non-CRO variables are used in both the Linux and the Windows target.
# WIN_WINDRES is used to compile the icon into an object file.

WIN_C     = mingw32-gcc.exe
WIN_CPP     = mingw32-g++.exe
WIN_LD      = mingw32-g++.exe
WIN_WINDRES = windres.exe

# change WIN_MINGDIR to your MinGW's "i586-..." directory.
# It should sit inside /usr or /usr/local.
# Maybe you don't have to change it, because $(CRO_LD) will know it by itself.
WIN_MINGDIR  = c:/mingw
WIN_MINGDIR_ARCH  = c:/mingw/lib/gcc/mingw32/4.8.1

WIN_CXXFLAGS = -Wall -W -nostdinc -D__NO_INLINE__ -O3
WIN_INCLUDES = -I$(WIN_MINGDIR)/include -I$(WIN_MINGDIR_ARCH)/include/c++ -I$(WIN_MINGDIR_ARCH)/include/c++/mingw32 -I$(WIN_MINGDIR_ARCH)/include/c++/backward -I$(WIN_MINGDIR_ARCH)/include -I$(WIN_MINGDIR_ARCH)/include-fixed -I$(WIN_MINGDIR)/mingw32/include 
#WIN_LIBS = -L$(WIN_MINGDIR)/lib -lalleg44 -mwindows -lenet -lpng -lz -lwinmm -lPocoNet -lPocoJSON -lPocoUtil -lPocoFoundation -liphlpapi -lws2_32 -lconfig++
WIN_LIBS = -lalleg44 -mwindows -lenet -lpng -lz -lwinmm -lPocoNet -lPocoJSON -lPocoUtil -lPocoFoundation -liphlpapi -lws2_32 -lconfig++

WIN_OBJDIR   = obj
WIN_BINDIR   = bin

WIN_CLIENT_BIN  = $(WIN_BINDIR)/winlix.exe

WIN_ICON_SRC    = $(SRCDIR)/icon.rc

WIN_CLIENT_OBJS = $(subst $(SRCDIR)/,$(WIN_OBJDIR)/,$(CLIENT_CSRC:%.c=%.o)) \
                  $(subst $(SRCDIR)/,$(WIN_OBJDIR)/,$(CLIENT_SRCS:%.cpp=%.o))
WIN_ICON_OBJ    = $(WIN_OBJDIR)/icon.res

#WIN_DEPGEN   = $(WIN_CPP) $(WIN_CXXFLAGS) $(WIN_INCLUDES) -MM
#WIN_DEPGEN   = $(WIN_CPP) $(WIN_CXXFLAGS) $(WIN_INCLUDES) 
#WIN_DEPDIR   = $(WIN_OBJDIR)

###############################################################################

.PHONY: all clean cross win

all: $(CLIENT_BIN) $(SERVER_BIN)

clean:
	$(RM) $(CLIENT_BIN)
	$(RM) $(SERVER_BIN)
	$(RM) $(OBJDIR)
	$(RM) $(DEPDIR)

cross: $(CRO_CLIENT_BIN) $(CRO_SERVER_BIN)

win: $(WIN_CLIENT_BIN)

###############################################################################

# Linux native compilation

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(Q)$(MKDIR) $(BINDIR)
	@echo Linking the game \`$(CLIENT_BIN)\' with \
		$(LDALLEG) $(LDENET) $(LDPNG) $(LDPOCO)
	$(Q)$(LD) $(CXXFLAGS) $(CPPFLAGS) $(LDALLEG) $(LDENET) $(LDPNG) $(LDCONFIG) $(LDPOCO) \
		$(CLIENT_OBJS) -o $(CLIENT_BIN) > /dev/null
	$(Q)$(STRIP) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS)
	$(Q)$(MKDIR) $(BINDIR)
	@echo Linking the server daemon \`$(SERVER_BIN)\' with \
		$(LDENET)
	$(Q)$(LD) $(CXXFLAGS) $(CPPFLAGS) $(LDENET) $(SERVER_OBJS) \
		-o $(SERVER_BIN) > /dev/null
	$(Q)$(STRIP) $(SERVER_BIN)

define MAKEFROMSOURCE
$(Q)$(MKDIR) `dirname $@` `dirname $(DEPDIR)/$*.d`
@echo $<
$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
@printf "%s/%s" `dirname $@` "`$(DEPGEN) $<`" > $(DEPDIR)/$*.d
endef

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(MAKEFROMSOURCE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(MAKEFROMSOURCE)

-include $(CLIENT_DEPS)
-include $(SERVER_DEPS)



###############################################################################

# Cross-compilation on Linux for Windows

$(CRO_CLIENT_BIN): $(CRO_CLIENT_OBJS) $(CRO_ICON_OBJ)
	$(Q)$(MKDIR) $(CRO_BINDIR)
	@echo Linking the cross-compiled game \`$(CRO_CLIENT_BIN)\' with \
		$(CRO_LDALLEG) $(CRO_LDENET) $(CRO_LDPNG)
	$(Q)$(CRO_LD) -o $(CRO_CLIENT_BIN) \
		$(CRO_CLIENT_OBJS) $(CRO_ICON_OBJ) \
		$(CRO_LDALLEG) $(CRO_LDENET) $(CRO_LDPNG) \
		> /dev/null
	$(Q)$(STRIP) $(CRO_CLIENT_BIN)

$(CRO_SERVER_BIN): $(CRO_SERVER_OBJS)
	$(Q)$(MKDIR) $(CRO_BINDIR)
	@echo Linking the cross-compiled server daemon \`$(CRO_SERVER_BIN)\' with \
		$(CRO_LDENET)
	$(Q)$(CRO_LD) -o $(CRO_SERVER_BIN) $(CRO_SERVER_OBJS) $(CRO_LDENET) \
		> /dev/null
	$(Q)$(STRIP) $(CRO_SERVER_BIN)

define CRO_MAKEFROMSOURCE
$(Q)$(MKDIR) `dirname $@` `dirname $(DEPDIR)/$*.d`
@echo $<
$(Q)$(CRO_CXX) $(CXXFLAGS) $(CRO_CPPFLAGS) -c $< -o $@
@printf "%s/%s" `dirname $@` "`$(DEPGEN) $<`" > $(DEPDIR)/$*.d
endef

$(CRO_OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CRO_MAKEFROMSOURCE)

$(CRO_OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CRO_MAKEFROMSOURCE)

$(CRO_ICON_OBJ): $(CRO_ICON_SRC)
	$(Q)$(MKDIR) `dirname $@`
	@echo $<
	$(Q)$(CRO_WINDRES) $< -O coff -o $(CRO_ICON_OBJ)

###############################################################################

# MinGW compilation in Windows


$(WIN_CLIENT_BIN): $(WIN_CLIENT_OBJS) $(WIN_ICON_OBJ)
	$(Q)$(MKDIR) $(WIN_BINDIR)
	@echo Linking the windows-compiled game \`$(WIN_CLIENT_BIN)\' with \
		$(WIN_LIBS)
	$(Q)$(WIN_LD) -o $(WIN_CLIENT_BIN) \
		$(WIN_CLIENT_OBJS) $(WIN_ICON_OBJ) \
		$(WIN_LIBS)  
#	$(Q)$(STRIP) $(WIN_CLIENT_BIN)


#define WIN_MAKEFROMSOURCE
#$(Q)$(MKDIR) `dirname $@` `dirname $(WIN_DEPDIR)/$*.d`
#@echo $<
#$(Q)$(WIN_CPP) $(WIN_CXXFLAGS) $(WIN_INCLUDES) -c $< -o $@
#@printf "%s/%s" `dirname $@` "`$(WIN_DEPGEN) $<`" > $(WIN_DEPDIR)/$*.d
#endef

#define WIN_MAKEFROMSOURCEC
#$(Q)$(MKDIR) `dirname $@` `dirname $(WIN_DEPDIR)/$*.d`
#@echo $<
#$(Q)$(WIN_C) $(WIN_CXXFLAGS) $(WIN_INCLUDES) -c $< -o $@
#@printf "%s/%s" `dirname $@` "`$(WIN_DEPGEN) $<`" > $(WIN_DEPDIR)/$*.d
#endef

define WIN_MAKEFROMSOURCE
$(Q)$(MKDIR) `dirname $@` `dirname $(WIN_OBJDIR)/$*.o`
@echo $<
$(Q)$(WIN_CPP) $(WIN_CXXFLAGS) $(WIN_INCLUDES) -c $< -o $@
endef

define WIN_MAKEFROMSOURCEC
$(Q)$(MKDIR) `dirname $@` `dirname $(WIN_OBJDIR)/$*.o`
@echo $<
$(Q)$(WIN_C) $(WIN_CXXFLAGS) $(WIN_INCLUDES) -c $< -o $@
endef

$(WIN_OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(WIN_MAKEFROMSOURCE)

$(WIN_OBJDIR)/%.o: $(SRCDIR)/%.c
	$(WIN_MAKEFROMSOURCEC)

$(WIN_ICON_OBJ): $(WIN_ICON_SRC)
	$(Q)$(MKDIR) `dirname $@`
	@echo $<
	$(Q)$(WIN_WINDRES) -J rc -O coff -i $< -o $(WIN_ICON_OBJ)
