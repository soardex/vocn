
default: all

TARGET=voc
CC=g++
RM=rm -f
CP=cp
SED=sed
CP_R=cp -r
RM_R=rm -rf
MKDIR_P=mkdir -p

OUTDIR=build
BINDIR=$(OUTDIR)/bin
OBJDIR=$(OUTDIR)/objects
RESDIR=$(OUTDIR)/assets
LUADIR=$(OUTDIR)/scripts
CNFDIR=$(OUTDIR)/configs
TSTDIR=$(OUTDIR)/tests
DEPDIR=$(OUTDIR)/objects/deps

DF=$(DEPDIR)/$(*F)

NCRDIR=ncres
INCDIR=-isystem /usr/include/bullet/ \
	   -I/usr/include/freetype2/
LIBDIR=-L/usr/lib64/

SRCDIR=voc

SRCS=\
	$(SRCDIR)/AppMain.cpp \
	$(SRCDIR)/EmperorSystem.cpp \
	$(SRCDIR)/SecondLife.cpp \
	$(SRCDIR)/imported/tinyobjloader/tiny_obj_loader.cpp \
	$(SRCDIR)/utils/Helpers.cpp \
	$(SRCDIR)/system/Renderer.cpp \
	$(SRCDIR)/system/EventHandler.cpp \
	$(SRCDIR)/system/ScriptManager.cpp \
	$(SRCDIR)/system/TextureManager.cpp \
	$(SRCDIR)/system/FontManager.cpp \
	$(SRCDIR)/system/PhysicsManager.cpp \
	$(SRCDIR)/system/SpriteManager.cpp

OBJS=$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
LIBS=-lm -lSDL2 -lGLEW -lGL -lGLU -lfreeimage -llua -lassimp \
	 -lLinearMath -lBulletDynamics -lBulletCollision -lBulletSoftBody \
	 -lfreetype -lpugixml

CFLAGS=-std=c++0x -g -Wall -Wextra -pedantic -fopenmp $(INCDIR)
LFLAGS=$(LIBDIR) $(LIBS)

all: directories populate $(TARGET)

$(OBJDIR)/%.o: %.c
	$(CC) -M $(CFLAGS) -c $^ -o $(DF).d
	$(CP) $(DF).d $(DF).P
	$(SED)  -e 's/#.*//' -e 's/^[^:]*: *//' \
		-e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DF).d >> $(DF).P 
	$(RM) $(DF).d
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/%.o: %.cpp
	$(CC) -M $(CFLAGS) -c $^ -o $(DF).d
	$(CP) $(DF).d $(DF).P
	$(SED)  -e 's/#.*//' -e 's/^[^:]*: *//' \
		-e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DF).d >> $(DF).P
	$(RM) $(DF).d
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(BINDIR)/$@ $^

directories: 
	$(MKDIR_P) $(OUTDIR)
	$(MKDIR_P) $(OBJDIR)
	$(MKDIR_P) $(OBJDIR)/$(SRCDIR)
	$(MKDIR_P) $(OBJDIR)/$(SRCDIR)/utils
	$(MKDIR_P) $(OBJDIR)/$(SRCDIR)/system
	$(MKDIR_P) $(OBJDIR)/$(SRCDIR)/imported
	$(MKDIR_P) $(OBJDIR)/$(SRCDIR)/imported/tinyobjloader
	$(MKDIR_P) $(DEPDIR)
	$(MKDIR_P) $(BINDIR)
	$(MKDIR_P) $(RESDIR)
	$(MKDIR_P) $(LUADIR)
	$(MKDIR_P) $(CNFDIR)
	$(MKDIR_P) $(TSTDIR)

populate:
	$(CP_R) $(NCRDIR)/assets $(OUTDIR)
	$(CP_R) $(NCRDIR)/configs $(OUTDIR)
	$(CP_R) $(NCRDIR)/scripts $(OUTDIR)

.PHONY: clean

clean:
	$(RM) $(OBJDIR)/*.{o,P,d} 
	$(RM) $(OBJDIR)/$(SRCDIR)/*.{o,P,d}
	$(RM) $(DEPDIR)/*.{o,P,d} 
	$(RM) $(BINDIR)/$(TARGET)
	$(RM_R) $(OUTDIR)

# DO NOT DELETE THIS LINE -- make depends needs it

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
