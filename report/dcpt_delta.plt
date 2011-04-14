set xtic scale 0 offset 0
set yrange [1.08:1.105]
set xrange [5:21]
set ytic nomirror
set xlabel "Number of deltas"
set ylabel "Speedup"
set border 3
set key off
set size .5,.5

set terminal postscript eps enhanced
set output "dcpt_delta_plt.eps"

set style data histogram

plot "dcpt_delta.dat" using 2

