set xr [0:5.5]
set xtic 1 nomirror
set ytic nomirror
set xlabel "Prefetch degree"
set ylabel "Speedup"
set border 3
set key off
set size .5,.5

set terminal postscript eps enhanced
set output "sequential_failover_speedup_plt.eps"

set style data histogram

plot "sequential_failover_speedup.dat" using 2
