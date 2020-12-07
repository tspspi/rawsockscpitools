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
	bin/rigolmso5000_timeseriesavg$(EXESUFFIX) \
	bin/siglentssg3021x_idn$(EXESUFFIX) \
	bin/siglentssg3021x_rfonoff$(EXESUFFIX) \
	bin/siglentssg3021x_setfrq$(EXESUFFIX) \
	bin/siglentssg3021x_setpow$(EXESUFFIX) \
	bin/siglentssa3021x_idn$(EXESUFFIX) \
	bin/siglentssa3021x_setfrq$(EXESUFFIX) \
	bin/siglentssa3021x_setspan$(EXESUFFIX) \
	bin/siglentssa3021x_avg$(EXESUFFIX) \
	bin/siglentssa3021x_querytrace$(EXESUFFIX)


OBJS=tmp/rigolmso5000.o \
	tmp/scpicommand.o \
	tmp/siglentssg3021x.o \
	tmp/siglentssa3021x.o

GENINCLUDES=include/labtypes.h

all: $(LIBS) $(TOOLS)

experiments: $(LIBS) bin/sweepSSG3021_SSA3021$(EXESUFFIX) bin/sweepSSG3021_RigolAVG$(EXESUFFIX)

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

tmp/siglentssa3021x.o: $(GENINCLUDES) include/siglent_ssa3021x.h src/siglentssa3021x.c

	$(CCOBJ) -o tmp/siglentssa3021x.o src/siglentssa3021x.c


# Tooling

bin/rigol5000_idn$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/rigolmso5000_idn/rigol5000_idn.c

	$(CCBIN) -o bin/rigol5000_idn$(EXESUFFIX) src/rigolmso5000_idn/rigol5000_idn.c $(CCBINSUFFIX)

bin/rigol5000_querywaveform$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/rigolmso5000_querywaveform/rigol5000_querywaveform.c

	$(CCBIN) -o bin/rigol5000_querywaveform$(EXESUFFIX) src/rigolmso5000_querywaveform/rigol5000_querywaveform.c $(CCBINSUFFIX)

bin/siglentssg3021x_idn$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_idn/siglentssg3021x_idn.c

	$(CCBIN) -o bin/siglentssg3021x_idn$(EXESUFFIX) src/siglentssg3021x_idn/siglentssg3021x_idn.c $(CCBINSUFFIX)

bin/siglentssg3021x_rfonoff$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_rfonoff/siglentssg3021x_rfonoff.c

	$(CCBIN) -o bin/siglentssg3021x_rfonoff$(EXESUFFIX) src/siglentssg3021x_rfonoff/siglentssg3021x_rfonoff.c $(CCBINSUFFIX)

bin/siglentssg3021x_setfrq$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_setfrq/siglentssg3021x_setfrq.c

	$(CCBIN) -o bin/siglentssg3021x_setfrq$(EXESUFFIX) src/siglentssg3021x_setfrq/siglentssg3021x_setfrq.c $(CCBINSUFFIX)

bin/siglentssg3021x_setpow$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssg3021x_setpow/siglentssg3021x_setpow.c

	$(CCBIN) -o bin/siglentssg3021x_setpow$(EXESUFFIX) src/siglentssg3021x_setpow/siglentssg3021x_setpow.c $(CCBINSUFFIX)

bin/siglentssa3021x_idn$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssa3021x_idn/siglentssa3021x_idn.c

	$(CCBIN) -o bin/siglentssa3021x_idn$(EXESUFFIX) src/siglentssa3021x_idn/siglentssa3021x_idn.c $(CCBINSUFFIX)

bin/siglentssa3021x_setfrq$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssa3021x_setfrq/siglentssa3021x_setfrq.c

	$(CCBIN) -o bin/siglentssa3021x_setfrq$(EXESUFFIX) src/siglentssa3021x_setfrq/siglentssa3021x_setfrq.c $(CCBINSUFFIX)

bin/siglentssa3021x_setspan$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssa3021x_setspan/siglentssa3021x_setspan.c

	$(CCBIN) -o bin/siglentssa3021x_setspan$(EXESUFFIX) src/siglentssa3021x_setspan/siglentssa3021x_setspan.c $(CCBINSUFFIX)

bin/siglentssa3021x_avg$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssa3021x_avg/siglentssa3021x_avg.c

	$(CCBIN) -o bin/siglentssa3021x_avg$(EXESUFFIX) src/siglentssa3021x_avg/siglentssa3021x_avg.c $(CCBINSUFFIX)

bin/siglentssa3021x_querytrace$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/siglentssa3021x_querytrace/siglentssa3021x_querytrace.c

	$(CCBIN) -o bin/siglentssa3021x_querytrace$(EXESUFFIX) src/siglentssa3021x_querytrace/siglentssa3021x_querytrace.c $(CCBINSUFFIX)

bin/rigolmso5000_timeseriesavg$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) src/rigolmso5000_timeseriesavg/rigol5000_timeseriesavg.c

	$(CCBIN) -o bin/rigolmso5000_timeseriesavg$(EXESUFFIX) src/rigolmso5000_timeseriesavg/rigol5000_timeseriesavg.c $(CCBINSUFFIX)

# Some experimental tasks

bin/sweepSSG3021_SSA3021$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) experiments/sweepSSG3021_SSA3021.c

	$(CCBIN) -o bin/sweepSSG3021_SSA3021$(EXESUFFIX) experiments/sweepSSG3021_SSA3021.c $(CCBINSUFFIX)

bin/sweepSSG3021_RigolAVG$(EXESUFFIX): bin/librawsockscpitools$(SLIBSUFFIX) $(GENINCLUDES) experiments/sweepSSG3021_RigolAVG.c

	$(CCBIN) -o bin/sweepSSG3021_RigolAVG$(EXESUFFIX) experiments/sweepSSG3021_RigolAVG.c $(CCBINSUFFIX)
