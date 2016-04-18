set terminal postscript portrait enhanced lw 2 "Helvetica" 14

set size 1.0, 0.66

#-------------------------------------------------------
set out "luna-throughput.eps"
#set title "Packet Loss Over Distance"
set xlabel "Distance (m)"
set xrange [0:200]
set ylabel "% Packet Loss --- average of 5 trials per distance"
set yrange [0:110]

plot "luna-ns3-sim.txt"  with lines title "WiFi Defaults"
