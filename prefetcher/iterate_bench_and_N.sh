# Hold stride constant, change N, output speedup for each N, when aborted by
# user (Ctrl-C) move on to next stride and repeat procedure.
issue_dir=`pwd`
framework=~/programs/m5/framework

extract_speedup() {
  echo ${bench}, ${N_VAL}: `cat output/${bench}/stats.txt | \
      grep system.switch_cpus_1.ipc_total | sed s/\ //g | \
      sed s/system\.switch_cpus_1\.ipc_total// | cut -d\# -f1` | \
      tee -a $issue_dir/N\ for\ speedup\ bench
  sleep 1
}

trap "loop=false" SIGINT

for bench in ammp art110 art470 wupwise swim applu galgel apsi \
  bzip2_source bzip2_graphic bzip2_program twolf
do
    cd $framework
    N_VAL=0
    loop=true
    while $loop
    do
        echo \#define N_VAL $N_VAL > prefetcher/nval.h

        export M5_CPU2000=lib/cpu2000

        cd m5
        scons -j2 NO_FAST_ALLOC=False EXTRAS=../prefetcher \
            ./build/ALPHA_SE/m5.opt

        cd ..
        ./m5/build/ALPHA_SE/m5.opt --remote-gdb-port=0 -re --trace-flags=Leif \
          --outdir output/$bench \
            m5/configs/example/se.py --detailed --checkpoint-dir=lib/cp \
          --checkpoint-restore=1000000000 --at-instruction --caches --l2cache \
          --standard-switch --warmup-insts=10000000 --max-inst=10000000 \
          --l2size=1MB --membus-width=8 --membus-clock=400MHz \
          --mem-latency=30ns --bench=${bench} \
          --prefetcher=on_access=true:policy=proxy

        extract_speedup

        N_VAL=$(($N_VAL+1))

        if [ $N_VAL -gt 10 ]; then loop=false; fi
    done
done