EXECS = getTMs getTMsInMargin sampleFailuresOfTopo getOptOSPFWeights applyOSPF

COMPILE = gcc
CFLAGS = -lm #-g


all: $(EXECS)

getTMs: topo_help.h getTMs.c
	$(COMPILE) $(CFLAGS) getTMs.c -o getTMs

getTMsInMargin: topo_help.h getTMsInMargin.c
	$(COMPILE) $(CFLAGS) getTMsInMargin.c -o getTMsInMargin

sampleFailuresOfTopo: topo_help.h sampleFailuresOfTopo.c
	$(COMPILE) $(CFLAGS) sampleFailuresOfTopo.c -o sampleFailuresOfTopo

getOptOSPFWeights: topo_help.h getOptOSPFWeights.c
	$(COMPILE) $(CFLAGS) getOptOSPFWeights.c -o getOptOSPFWeights

applyOSPF: topo_help.h applyOSPF.c
	$(COMPILE) $(CFLAGS) applyOSPF.c -o applyOSPF

clean:
	rm $(EXECS) core.* *~
