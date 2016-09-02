
TARGET   = ../Release/Plugins/Plugin_SubProcess.so

SOURCES  = SubProcess_Manager.cpp \
           SubProcess_Thread.cpp \
           SubProcess_Queue.cpp \
           Plugin_SubProcess.cpp 

OBJECTS  = $(SOURCES:.cpp=.o)

LDADD    = ../Library_MMDAgent/lib/MMDAgent.a \
           ../Library_MMDFiles/lib/MMDFiles.a \
           ../Library_GLFW/lib/GLFW.a \
           ../Library_GLee/lib/GLee.a \
           ../Library_Bullet_Physics/lib/Bullet_Physics.a \
           ../Library_libpng/lib/libpng.a \
           ../Library_JPEG/lib/JPEG.a \
           ../Library_zlib/lib/zlib.a

CXX      = gcc
AR       = ar
CXXFLAGS = -Wall -g -O3 -fomit-frame-pointer -fPIC \
           -shared \
           -DMMDAGENT
INCLUDE  = -I ../Library_Bullet_Physics/include \
           -I ../Library_GLee/include \
           -I ../Library_GLFW/include \
           -I ../Library_MMDFiles/include \
           -I ../Library_MMDAgent/include 

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LDADD)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDADD) -o $(TARGET) \
	-lGLU -lGL -lX11

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(<:.cpp=.o) -c $<

clean:
	rm -f $(OBJECTS) $(TARGET)
