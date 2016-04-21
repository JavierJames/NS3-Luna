#set terminal postscript portrait enhanced lw 2 "Helvetica" 14

#set size 1.0, 0.66

#-------------------------------------------------------

set xlabel "packet size (bytes)"
set xrange [0:]
set ylabel "% Avg throughput --- average of 5 trials per distance"
set yrange [0:]

#set style histogram errorbars gap 2 lw 1
#plot "perfDelay.txt" u 1:2:3 w hist

plot "perfDelay.txt" with yerrorbar 
replot "perfDelay11.txt" with yerrorbar 

set term png
set output "luna-perfDelay.png"
replot
set term x11
