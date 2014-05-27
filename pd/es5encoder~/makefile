current: pd_linux

clean: ; rm -f *.pd_linux *.o

# ----------------------- LINUX i386 -----------------------

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Werror -fPIC \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I../../pd-0.45-4/src

pd_linux:
	g++ $(LINUXCFLAGS) $(LINUXINCLUDE) -o main.o -c main.cpp
	g++ -shared -o es5encoder~.pd_linux main.o -lc -lm
	strip --strip-unneeded es5encoder~.pd_linux

# ----------------------- NT -----------------------

pd_nt: es5encoder~.dll

.SUFFIXES: .obj .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo /wd4091
VC="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"

PDNTINCLUDE = /I. /I\tcl\include /I..\..\..\..\3rdparty\pd\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\oldnames.lib \
	..\..\..\..\3rdparty\pd-0.45-4.msw\bin\pd.lib 

# override explicitly for tilde objects like this:
es5encoder~.dll: main.cpp; 
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c main.cpp
	link /dll /export:es5encoder_tilde_setup main.obj $(PDNTLIB) /OUT:es5encoder~.dll
