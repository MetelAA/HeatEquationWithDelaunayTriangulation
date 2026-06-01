set term qt size 400,400
set xlabel "Time"
set ylabel "Difference"
set title "Max temperature difference"
plot "../Plots/max_diff_data.txt" using 1:2 with lines t ""
pause mouse close "Click to quit"