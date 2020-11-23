SLIBSUFFIX=.a
DYNLIBSUFFIX=.so
EXESUFFIX=

CC=clang
CCOBJ=$(CC) -Wall -ansi -std=c99 -Werror -pedantic -c -fpic
CCSO=$(CC) -shared -fpic

CCBIN=$(CC) -Wall -ansi -std=c99 -Werror -pedantic -L./bin/
CCBINSUFFIX=bin/librawsockscpitools$(SLIBSUFFIX)

LIBS=bin/librawsockscpitools$(SLIBSUFFIX) \
	bin/librawsockscpitools$(DYNLIBSUFFIX)

TOOLS=bin/rigol5000_idn$(EXESUFFIX) \
	bin/rigol5000_querywaveform$(EXESUFFIX) \
	bin/siglentssg3021x_idn$(EXESUFFIX) \
	bin/siglentssg3021x_rfonoff$(EXESUFFIX)

OBJS=tmp/rigolmso5000.o \
	tmp/scpicommand.o \
	tmp/siglentssg3021x.o

GENINCLUDES=include/labtypes.h

all: $(LIBS) $(TOOLS)


bin/librawsockscpitools$(SLIBSUFFIX): $(OBJS)

	ar -rv bin/librawsockscpitools$(SLIBSUFFIX) $(OBJS)

bin/librawsockscpitools$(DYNLIBSUFFIX): $(OBJS)

	$(CCSO) -o bin/librawsockscpitools$(DYNLIBSUFFIX) $(OBJS)

# Library objects

tmp/scpicommand.o: $(GENINCLUDES) src/scpicommand.c

	$(CCOBJ) -o tmp/scpicommand.o src/scpicommand.c

tmp/rigolmso5000.o: $(GENINCLUDES) include/rigolmso5000.h src/rigolmso5000.c

	$(CCOBJ) -o tmp/rigolmso5000.o src/rigolmso5000.c

tmp/siglentssg3021x.o: $(GENINCLUDES) include/siglent_ssg3021x.h src/siglentssg3021x.c

	$(CCOBJ) -o tmp/siglentssg3021x.o src/siglentssg3021x.c

# Tooling

bin/rigol5000_idn$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/rigolmso5000_idn/rigol5000_idn.c

	$(CCBIN) -o bin/rigol5000_idn$(EXESUFFIX) src/rigolmso5000_idn/rigol5000_idn.c $(CCBINSUFFIX)

bin/rigol5000_querywaveform$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/rigolmso5000_querywaveform/rigol5000_querywaveform.c

	$(CCBIN) -o bin/rigol5000_querywaveform$(EXESUFFIX) src/rigolmso5000_querywaveform/rigol5000_querywaveform.c $(CCBINSUFFIX)

bin/siglentssg3021x_idn$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_idn/siglentssg3021x_idn.c

	$(CCBIN) -o bin/siglentssg3021x_idn$(EXESUFFIX) src/siglentssg3021x_idn/siglentssg3021x_idn.c $(CCBINSUFFIX)

bin/siglentssg3021x_rfonoff$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_rfonoff/siglentssg3021x_rfonoff.c

	$(CCBIN) -o bin/siglentssg3021x_rfonoff$(EXESUFFIX) src/siglentssg3021x_rfonoff/siglentssg3021x_rfonoff.c $(CCBINSUFFIX)
