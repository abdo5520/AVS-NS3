#!/bin/bash
for c in {1..10..1}
do
 echo $c
NS_GLOBAL_VALUE="RngRun=$c" ./waf --run NoDistanceModel5minAvgVarianceAlgorithm > outControl
echo $0_RngRun$c
bash results.bash outControl > $0_RngRun$c
done

