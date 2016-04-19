set terminal postscript portrait enhanced lw 2 "Helvetica" 14

set size 1.0, 0.66

#-------------------------------------------------------
set out "luna-throughput.eps"
#set title "Packet Loss Over Distance"
set xlabel "Avg throughput (m)"
set xrange [1:16]
set ylabel "% Avg throughput --- average of 5 trials per distance"
set yrange [0:5000]

plot "perfThroughputAvg.txt"  with lines title "Avg Throughput"
