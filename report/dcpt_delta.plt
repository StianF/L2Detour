set xtic scale 0 offset 1
set ytic nomirror
set xlabel "Number of deltas"
set ylabel "Speedup"
set border 3
set key off
set size .5,.5

set terminal postscript eps enhanced
set output "dcpt_delta_plt.eps"

set style data histogram

plot "dcpt_delta.dat" using 2:xtic(1) ti col
