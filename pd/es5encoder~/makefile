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

