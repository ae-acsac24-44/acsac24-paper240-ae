#!/bin/bash

SRV=$1
TEST="all"
RES="results"
RESFILE=""
OUTPUT=""
REPTS=0
LOG="log"
cnt=0

usage() {
    echo "Usage: $0 SERVER_IP [-t hackbench|kernbench|netperf|apache|memcached] [-n repts] [-o output]"
	exit 1
}

if [ -z "$SRV" ]; then
	usage
	exit
fi

shift
while [[ $# -gt 0 ]]; do
	case "$1" in
    	-n)
            if [[ "$2" =~ ^[0-9]+$ ]]; then
                REPTS=$2
				shift 2
            else
				echo "Error: Repetitions should be a positive integer"
				usage
            fi
            ;;
        -o)
            OUTPUT=$2
			shift 2
            ;;
        -t)
            TEST=$2
			shift 2
            ;;
        -*)
			echo "Error: Unknown option: $1" >&2
            usage
            ;;
        *)
            usage
            ;;
    esac
done

avg() {
    local input="$1"
    local output="$2"
    local sum=0
    local count=0

    while IFS= read -r line; do
	if [[ "$line" =~ ^-?[0-9]+(\.[0-9]+)?$ ]]; then
            sum=$(echo "$sum + $line" | bc)
            count=$((count + 1))
	fi
    done < "$input"

    # Calculate and print the average
    if [ "$count" -gt 0 ]; then
        average=$(echo "scale=4; $sum / $count" | bc)
        echo "Average: $average" >> $output
    fi
}

run_netperf() {
	echo "Running netperf performance of $SRV"
	if [[ -z "$OUTPUT" ]]; then
		LOGFILE="$LOG/netperf_log.$cnt"
	else
		LOGFILE="$LOG/netperf_log.$OUTPUT"
	fi
	echo "The raw benchmark data will be stored in $LOGFILE."

	./prep-netperf.sh $SRV
	sleep 1
	for _TEST in TCP_STREAM TCP_MAERTS TCP_RR; do
		if [[ $REPTS != 0 ]]; then
			./netperf.sh $SRV $_TEST $REPTS | tee -a $LOGFILE 2>/dev/null
		else
			./netperf.sh $SRV $_TEST | tee -a $LOGFILE 2>/dev/null
		fi	
		sleep 1
		if [[ "$_TEST" == "TCP_RR" ]]; then
			echo "Netperf $_TEST Result (Req/Res per sec):" >> $RESFILE
		else
			echo "Netperf $_TEST Result (Mbps):" >> $RESFILE
		fi
		tail -n +2 netperf.txt >> $RESFILE
		avg netperf.txt $RESFILE
		rm netperf.txt
	done
}

run_apache() {
	echo "Running apache performance of $SRV"
	if [[ -z "$OUTPUT" ]]; then
		LOGFILE="$LOG/apache_log.$cnt"
	else
		LOGFILE="$LOG/apache_log.$OUTPUT"
	fi
	echo "The raw benchmark data will be stored in $LOGFILE."

	./prep-apache.sh $SRV
	sleep 1
	if [[ "$REPTS" != "0" ]]; then
		./apache.sh $SRV $REPTS | tee -a $LOGFILE 2>/dev/null
	else
		./apache.sh $SRV | tee -a $LOGFILE 2>/dev/null
	fi

	echo "Apache Result (Requests per second):" >> $RESFILE
	cat apache.txt >> $RESFILE
	avg apache.txt $RESFILE
	rm apache.txt
}

run_memcached() {
	echo "Running memcached performance of $SRV"
	if [[ -z "$OUTPUT" ]]; then
		LOGFILE="$LOG/memcached_log.$cnt"
	else
		LOGFILE="$LOG/memcached_log.$OUTPUT"
	fi
	echo "The raw benchmark data will be stored in $LOGFILE."
	
	./prep-memcached.sh $SRV
	sleep 1
	if [[ $REPTS != 0 ]]; then
		./memcached.sh $SRV $REPTS | tee -a $LOGFILE 2>/dev/null
	else
		./memcached.sh $SRV | tee -a $LOGFILE 2>/dev/null
	fi

	echo "Memcached Result (Ops per sec):" >> $RESFILE
	tempfile=$(mktemp)
	cat memcached.txt | awk '/Totals/ {print $2}' > $tempfile
	cat $tempfile >> $RESFILE
	avg $tempfile $RESFILE
	rm $tempfile memcached.txt
}

run_hackbench() {
	echo "Running hackbench performance of $SRV"
	if [[ -z "$OUTPUT" ]]; then
		LOGFILE="$LOG/hackbench_log.$cnt"
	else
		LOGFILE="$LOG/hackbench_log.$OUTPUT"
	fi
	echo "The raw benchmark data will be stored in $LOGFILE."

	./hack.sh $SRV | tee -a $LOGFILE 2>/dev/null
	./grab-hack.sh $SRV
	echo "Hackbench Result (sec):" >> $RESFILE
	cat hackbench.txt >> $RESFILE
	avg hackbench.txt $RESFILE
	rm hackbench.txt
}

run_kernbench() {
	echo "Running kernbench performance of $SRV"
	if [[ -z "$OUTPUT" ]]; then
		LOGFILE="$LOG/kernbench_log.$cnt"
	else
		LOGFILE="$LOG/kernbench_log.$OUTPUT"
	fi
	echo "The raw benchmark data will be stored in $LOGFILE."

	./kern.sh $SRV | tee -a $LOGFILE 2>/dev/null
	./grab-kern.sh $SRV
	echo "Kernbench Result (sec):" >> $RESFILE
	cat kernbench.txt >> $RESFILE
	avg kernbench.txt $RESFILE
	rm kernbench.txt
}

run_all() {
	run_kernbench
	run_hackbench
	run_netperf
	run_apache
	run_memcached
}

if [ ! -d $RES ]; then
	mkdir $RES
fi

if [ ! -d $LOG ]; then
	mkdir $LOG
fi

if [[ -z $OUTPUT ]]; then
	for file in "$RES"/benchmark_results.*; do
		if [[ "$file" =~ benchmark_results\.([0-9]+)$ ]]; then
        	num="${BASH_REMATCH[1]}"
        	if [ "$num" -ge "$cnt" ]; then
            	cnt=$((num + 1))
        	fi
    	fi
	done
	RESFILE="$RES/benchmark_results.$cnt"
else
	RESFILE="$RES/$OUTPUT"
fi

echo "The benchmark result will be stored at $RESFILE."

if [ "$TEST" == "all" ]; then
    echo "Running all benchmark performance tests on $SRV..."
    echo ""
    run_all
else
    benchmark="run_$TEST"
    
    if declare -f "$benchmark" > /dev/null; then
        $benchmark
    else
		usage
    fi
fi
