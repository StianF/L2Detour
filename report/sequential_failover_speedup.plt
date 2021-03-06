set xtic scale 0 offset 1
set ytic nomirror
set xlabel "Prefetch degree"
set ylabel "Speedup"
set border 3
set key off
set size .5,.5

set terminal postscript eps enhanced
set output "sequential_failover_speedup_plt.eps"

set style data histogram

plot "sequential_failover_speedup.dat" using 2:xtic(1)
