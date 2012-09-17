#!/bin/bash
clear
echo $0 $1


echo "Current Level Statistics"
grep "Current Video Level is" -c $1
grep "Current Video Level is" $1
echo "Average ReTxCounter Values"
grep "Avergae ReTxCounter Now NOW NOW" -c $1
grep "Avergae ReTxCounter Now NOW NOW" $1
echo "The Number the CalcAverages function has been called within the MAC layer (Immediate Average Value Inside the MAC LAYER=)"
grep "Immediate Average Value Inside the MAC LAYER=" -c $1
echo "How many TIME OF RESET The Monitor Statistics = , in different words how many time the Monitoring Window Expiration detected"
grep "RESET STATISTICS AT =" -c $1
grep "Total Playable Video Frames" $1
echo "REBUFFERING " 
grep "REBUFFERING.........REBUFFERING.........REBUFFERING.........REBUFFERING........" -c $1
echo "Not Applicable" 
grep "Not Applicable" -c $1
echo "Stay The Same"
grep "STAY THE SAME " -c $1
echo "set " 
grep "set" -c $1
echo "set Up" 
grep "set Up " -c $1
echo "set Down" 
grep "set Down" -c $1
echo "Level Changed " 
grep "Level Changed" -c $1

echo "How Long Level 0"
grep "Current Video Level is  0" -c $1
echo "How Long Level 1 "
grep "Current Video Level is  1" -c $1
echo "How Long Level 2"
grep "Current Video Level is  2" -c $1
echo "How Long Level 3"
grep "Current Video Level is  3" -c $1
echo "How Long Level 4"
grep "Current Video Level is  4" -c $1
