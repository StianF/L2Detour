for f in `seq 0 $1`
do echo $f: `sh juggle_numbers.sh $1 | tail -n+2 | sh harmonic_avg.sh $1 $f`
done