set auto x
set xlabel "Table size"
set ylabel "Speedup"
set xtic 1 nomirror
set ytic nomirror
set border 3
set xtic rotate by -45 scale 0

set terminal postscript eps enhanced
set output "dcpt_delta_plt.eps"

set style data histogram

plot "dcpt_delta.dat" using 2:xtic(1) ti col
