set terminal png
set xlabel "Time (in seconds)"
set ylabel "Congestion Window"
set output "Congestion-NewReno.png"
set title "Congestion Window Calculation NewReno"
plot "q1-NewReno.cwnd" using 1:3 with linespoints title "New Congesiton"


set output "Congestion-Vegas.png"
set title "Congestion Window Calculation Vegas"
plot "q1-Vegas.cwnd" using 1:3 with linespoints title "New Congesiton"


set output "Congestion-Veno.png"
set title "Congestion Window Calculation Veno"
plot "q1-Veno.cwnd" using 1:3 with linespoints title "New Congesiton"


set output "Congestion-HighSpeed.png"
set title "Congestion Window Calculation HighSpeed"
plot "q1-HighSpeed.cwnd" using 1:3 with linespoints title "New Congesiton"
