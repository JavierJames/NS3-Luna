#!/bin/sh

DATABASE_NAME="luna-ns3"


#DISTANCES="25 50 75 100 125 145 147 150 152 155 157 160 162 165 167 170 172 175 177 180"
#TRIALS="1 2 3 4 5"
#nSAT="1 2 5 10 15 20 25 30 35 40 45 50"

#TRIALS="1 2"
#nSAT="1 2"

TRIALS="1 2  "
nSAT="1 2 3"

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

#if [ -e ../../data.db ]
#if [ -e ../../../luna-ns3-sim.db ]
#then
#  echo "Kill data.db? (y/n)"
#  read ANS
#  if [ "$ANS" = "yes" -o "$ANS" = "y" ]
#  then
#    echo Deleting database
    #rm ../../data.db
#    rm ../../../luna-ns3-sim.db
#  fi
#fi

for trial in $TRIALS
do
  for nsats in $nSAT
  do
    echo Trial $trial, number of satellites $nsats
    #../../waf --run "wifi-luna --format=db --distance=$nsats --run=run-$distance-$trial"
    ../waf --run "luna --nSat=$nsats --format=db --run=run-$nsats-$trial"
  done
done


#mv ../../data.db .
#mv ../../../luna-ns3-sim.db .

#CMD="select exp.input,avg(100-((rx.value*100)/tx.value)) \
#    from Singletons rx, Singletons tx, Experiments exp \
#    where rx.run = tx.run AND \
#          rx.run = exp.run AND \
#          rx.variable='receiver-rx-packets' AND \
#          tx.variable='sender-tx-packets' \
#    group by exp.input \
#    order by abs(exp.input) ASC;"

#sqlite3 -noheader luna-ns3-sim.db "$CMD" > luna-ns3-sim.data
#sed -i.bak "s/|/   /" luna-ns3-sim.data
#rm luna-ns3-sim.bak
#gnuplot luna-ns3-sim.gnuplot

echo "Done; data in wifi-default.data, plot in wifi-default.eps"
