#!/bin/bash

FLOW=2
NODEARCCUT=1
OTAHOPLINK=2
ETAHOPLINK=2
HOPLINK=1
HOPEND=1
HFLOW=2
GHOPLINK=2
CUT=2

./STPRBH --file ../instances/$1 --outstat "./results/$1-$FLOW-$NODEARCCUT-$OTAHOPLINK-$ETAHOPLINK-$HOPLINK-$HOPEND-$HFLOW-$GHOPLINK-$CUT.stat" -T 1 --cplex.exportmodel 0 --cplex.lprelaxation 0 --sep.flow $FLOW --sep.nodearccut $NODEARCCUT --sep.otahoplink $OTAHOPLINK --sep.etahoplink $ETAHOPLINK --sep.hoplink $HOPLINK --sep.hopend $HOPEND --sep.flowhop $HFLOW --sep.ghoplink $GHOPLINK --sep.cut $CUT --cplex.cplexcuts 1 -t 1000 --sep.tailoff 1e-5

