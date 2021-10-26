set terminal png
set xlabel "Time (in seconds)"
set ylabel "Congestion Window Size"

list=system('ls -1B q2-a*.cwnd')
do for [file in list]{
  set title "Congestion window size vs time graph for ".file
  set output file.".png"
  plot file using 1:3 with linespoints title "New Congestion"
}
