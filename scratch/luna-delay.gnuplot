#set terminal postscript portrait enhanced lw 2 "Helvetica" 14

#set size 1.0, 0.66

#-------------------------------------------------------

set xlabel "packet size (bytes)"
set xrange [10:170]
set ylabel "% Avg Delay --- average of 5 trials per distance"
set yrange [0:]

#set style histogram errorbars gap 2 lw 1
#plot "perfDelay.txt" u 1:2:3 w hist

#plot "perfDelayStats.txt" with yerrorbar 
#replot "perfDelayStats.txt" with yerrorbar #2nd file 

plot "DsssRate1Mbps-Stats.txt" with yerrorbar #1st file
replot "DsssRate2Mbps-Stats.txt" with yerrorbar #2nd file 
replot "DsssRate5_5Mbps-Stats.txt" with yerrorbar #3rd file 
replot "DsssRate11Mbps-Stats.txt" with yerrorbar #4th file 


set term png
set output "luna-perfDelay.png"
replot
set term x11
