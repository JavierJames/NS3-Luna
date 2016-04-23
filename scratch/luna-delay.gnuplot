#set terminal postscript portrait enhanced lw 2 "Helvetica" 14

#set size 1.0, 0.66

#-------------------------------------------------------

#set the output 
set term png
set output "luna-perfDelay.png"
#set term x11

#axis property
#set xlabel "packet size (bytes)"
#set xrange [10:170]
#set ylabel "% Avg Delay --- average of 5 trials per distance"
#set yrange [0:]



#configure plot window & plot  
if (!exists("MP_LEFT"))   MP_LEFT = .1
if (!exists("MP_RIGHT"))  MP_RIGHT = .95
if (!exists("MP_BOTTOM")) MP_BOTTOM = .1
if (!exists("MP_TOP"))    MP_TOP = .9
if (!exists("MP_GAP"))    MP_GAP = 0.05




#set style histogram errorbars gap 2 lw 1
#plot "perfDelay.txt" u 1:2:3 w hist

#plot "perfDelayStats.txt" with yerrorbar 
#replot "perfDelayStats.txt" with yerrorbar #2nd file 

#plot "DsssRate1Mbps-Stats.txt" with yerrorbar #1st file
#replot "DsssRate2Mbps-Stats.txt" with yerrorbar #2nd file 
#replot "DsssRate5_5Mbps-Stats.txt" with yerrorbar #3rd file 
#replot "DsssRate11Mbps-Stats.txt" with yerrorbar #4th file 


#replot


set output "DsssRate1Mbps-Stats.png"
plot "DsssRate1Mbps-Stats.txt" with yerrorbar #2nd file 
set output "DsssRate2Mbps-Stats.png"
plot "DsssRate2Mbps-Stats.txt" with yerrorbar #2nd file 
set output "DsssRate5_5Mbps-Stats.png"
plot "DsssRate5_5Mbps-Stats.txt" with yerrorbar #2nd file 
set output "DsssRate11Mbps-Stats.png"
plot "DsssRate11Mbps-Stats.txt" with yerrorbar #2nd file 


