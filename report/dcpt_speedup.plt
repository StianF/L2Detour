set auto x
set xlabel "Program"
set ylabel "Speedup"
set xtic 1 nomirror
set ytic nomirror
set border 3
set xtic rotate by -45 scale 0

set terminal postscript eps enhanced
set output "dcpt_speedup_plt.eps"

set style data histogram

plot "dcpt_speedup.dat" using 3:xtic(1) ti col fs solid 0.00, '' u 2 ti col fs solid 0.50 lt 1
