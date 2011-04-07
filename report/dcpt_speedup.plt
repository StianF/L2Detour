set auto x
set xlabel "Program"
set ylabel "Speedup"
set xtic 1 nomirror
set ytic nomirror
set border 3
set key off
set xtic rotate by -45 scale 0

set terminal postscript eps enhanced
set output "dcpt_speedup_plt.eps"

set style data histogram

plot "dcpt_speedup.dat" using 2:xtic(1)
