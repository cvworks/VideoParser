# common.mk is meant to be use by including it in related
# make files of the ShapeMatcher project. In this file, it is
# assumed that the variables DAG_MATCHER_BASE and LEDAROOT
# are defined either in the parent make file or in the environment. 
# In addition, the variables LIB_NAME and APP_NAME are assumed to be
# set in the parent make file. eg, LIB_NAME = DAGMatch, APP_NAME = testDML

# Choose a value for ARCH from {Windows, Linux}, choose a value for 
# MODE from {Release, Debug}. Do not leave blank spaces after the value.

ARCH = Windows
MODE = Debug
#MODE = Release

# Set library paths

DAG_MATCHER_ROOT = $(DAG_MATCHER_BASE)/dagmatcher
SHAPE_MATCHER_ROOT = $(DAG_MATCHER_BASE)/ShapeMatcher
NEWMAT_ROOT = $(DAG_MATCHER_BASE)/Newmat
FLUX_SKELETON_ROOT = $(DAG_MATCHER_BASE)/FluxSkeleton
AFMM_SKELETON_ROOT = $(DAG_MATCHER_BASE)/AFMMSkeleton
ANN_ROOT = $(DAG_MATCHER_BASE)/ann_1.1
VC_PROJECT = $(DAG_MATCHER_BASE)/ShapeMatcherVC

# Set variables that depend on the ARCHITECURE

ifeq "$(ARCH)" "Windows"
	SM_BIN_ROOT = ..\bin\Windows\$(MODE)
	HNSRTREE_ROOT = $(DAG_MATCHER_BASE)/HnSRTree-2.0beta5a
	GLUT_ROOT = $(DAG_MATCHER_BASE)/glut-3.7.6-bin
	#SYS_LIBS = .
	
	VS_FLAGS = -EHsc -W3 -D_CRT_SECURE_NO_DEPRECATE #-RTC1 -Wp64 
	
	CXXFLAGS = -nologo $(VS_FLAGS) -GR -D_LIB -D_CONSOLE -DLEDA_DLL -DWIN32_LEAN_AND_MEAN
	BASIC_DEBUG_CXXFLAGS = -Od -Zi -MDd -DWIN32 -D_CRTDBG_MAP_ALLOC -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
	BASIC_RELEASE_CXXFLAGS = -O2 -GL -MD -DWIN32 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 

	OBJFILE = %.obj
	LIB_FULL_NAME = $(LIB_NAME).lib
	#APP_FULL_NAME = $(APP_NAME).exe
	
	ifeq "$(MODE)" "Debug"
		APP_FULL_NAME = $(APP_NAME)_dbg.exe
	else
		APP_FULL_NAME = $(APP_NAME)_rel.exe
	endif

	ARFLAGS = -nologo -out:$(LIB_FULL_NAME)
	LDFLAGS = -nologo /FORCE:MULTIPLE /SUBSYSTEM:CONSOLE /MACHINE:X86
	POST_BUILD_STEP = mt.exe -nologo -manifest $(APP_FULL_NAME).manifest -outputresource:$(APP_FULL_NAME)
	
	ifeq "$(MODE)" "Debug"
		LDFLAGS += /DEBUG
	else
		LDFLAGS += /OPT:REF /OPT:ICF /LTCG
		#LDFLAGS += /OPT:REF /OPT:ICF /LTCG:PGINSTRUMENT
		#LDFLAGS += /OPT:REF /OPT:ICF /LTCG:PGOPTIMIZE
		ARFLAGS += /LTCG
	endif
	
	L = LIBPATH:
	OUT = OUT:
	CXX = cl
	CC = cl
	AR = lib
	LINK = link
	RM = cmd /C del
	MV = cmd /C move
	CP = cmd /C copy
else
	SM_BIN_ROOT = ../bin/Linux/$(MODE)
	HNSRTREE_ROOT = $(DAG_MATCHER_BASE)/HnSRTree-2.0beta5b
	GLUT_ROOT = .
	#SYS_LIBS = /usr/X11R6/lib
	
	CXXFLAGS = 
	BASIC_DEBUG_CXXFLAGS = -g
	BASIC_RELEASE_CXXFLAGS = -O3

	#ifeq "$(MODE)" "Debug"
	#	LDFLAGS = 
	#else
	#	LDFLAGS = -static
	#endif

	L = L
	LINK = $(CXX)
	MV = mv
	CP = cp
	OUT = o 
	OBJFILE = %.o
	LIB_FULL_NAME = lib$(LIB_NAME).a
	APP_FULL_NAME = $(APP_NAME)
	ARFLAGS = -cr $(LIB_FULL_NAME)
	POST_BUILD_STEP =
endif

# Set variables that depend on the compilation MODE

ifeq "$(MODE)" "Debug"
	BASIC_CXXFLAGS = $(BASIC_DEBUG_CXXFLAGS) -D_DEBUG -D_HNSRTIMP=""
else
	BASIC_CXXFLAGS = $(BASIC_RELEASE_CXXFLAGS) -DNDEBUG -D_HNSRTIMP=""
endif

CXXFLAGS += $(BASIC_CXXFLAGS)

