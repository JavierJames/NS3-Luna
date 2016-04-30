#!/bin/sh

DATABASE_NAME="luna-ns3"


#DISTANCES="25 50 75 100 125 145 147 150 152 155 157 160 162 165 167 170 172 175 177 180"
#TRIALS="1 2 3 4 5"
#nSAT="1 2 5 10 15 20 25 30 35 40 45 50"


TRIALS="1 2 3"
nSAT="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"
#nSAT="1 2"

echo Luna NS3 Simulation

pCheck=`which sqlite3`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires sqlite3 (wifi-example-sim does not)."
  exit 255
fi

pCheck=`which gnuplot`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires gnuplot (wifi-example-sim does not)."
  exit 255
fi

pCheck=`which sed`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires sed (wifi-example-sim does not)."
  exit 255
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/
#if [ -e "perfThroughputAvg.txt" ]
#then
#  echo "Kill perfThroughputAvg.txt? (y/n)"
#  read ANS
#  if [ "$ANS" = "yes" -o "$ANS" = "y" ]
#  then
#    echo Deleting database
#    rm ./perfThroughput.txt
#    rm ./perfThroughputAvg.txt
#  fi
#fi





#for trial in $TRIALS
#do
#  for nsats in $nSAT
#  do
#    echo Trial $trial, number of satellites $nsats
#    ../waf --run "luna --nSat=$nsats --format=db --run=run-$nsats-$trial"
#  done
#done



if [ -e "DsssRate1Mbps.txt" ] ||[ -e "DsssRate2Mbps.txt" ] || [ -e "DsssRate5_5Mbps.txt" ] || [ -e "DsssRate11Mbps.txt" ]
then
  echo "Kill previous run dataset? (y/n)"
  read ANS
  if [ "$ANS" = "yes" -o "$ANS" = "y" ]
  then
    echo Deleting database
    rm ./DsssRate1Mbps.txt
    rm ./DsssRate1Mbps-Stats.txt
    rm ./DsssRate1Mbps-Stats.png
    rm ./DsssRate2Mbps.txt
    rm ./DsssRate2Mbps-Stats.txt
    rm ./DsssRate2Mbps-Stats.png
    rm ./DsssRate5_5Mbps.txt
    rm ./DsssRate5_5Mbps-Stats.txt
    rm ./DsssRate5_5Mbps-Stats.png
    rm ./DsssRate11Mbps.txt
    rm ./DsssRate11Mbps-Stats.txt
    rm ./DsssRate11Mbps-Stats.png
  fi
fi

#Perofmance simulation: Delay 
#parameters: Datalink speed 
#            Packet size 
TRIALS="1 2 3 4 5"
PACKETSIZE="20 24 38 50 60 80 160"
DATARATE="DsssRate1Mbps DsssRate2Mbps DsssRate5_5Mbps DsssRate11Mbps"

for trial in $TRIALS
do
  for datarate in $DATARATE
  do
    for packetsize in $PACKETSIZE
    do
      echo Trial $trial, number of satellites $nsats
      ../waf --run "luna --packetSize=$packetsize  --dataRate=$datarate --format=db --run=run-$datarate-$packetsize-$trial"
    done
  done
done  

gnuplot luna-delay.gnuplot
echo "Done; dataset in DsssRatexxMbps.txt [parameter,trial1, trail2, ..] , statistic in DsssRatexxMbps-Stats.txt [parameter,mean,standard deviation], plot in luna-perfDelay.png"

#gnuplot luna-throughput.gnuplot

