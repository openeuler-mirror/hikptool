#!/bin/bash

bw_stats_field=("mac_tx_total_oct_num" "mac_rx_total_oct_num")
declare -A start_stats
declare -A end_stats

init_start_stats(){
	local device=$1
	stats=$(ethtool -S "$device")

	for((i=0; i<${#bw_stats_field[@]};i++)); do
		start_stats[${bw_stats_field[i]}]=$(echo "$stats"|grep "${bw_stats_field[i]}"| awk '{print $2}')
	done
}

bw_end_stats(){
	local device=$1
	stats=$(ethtool -S "$device")

	for((i=0;i<${#bw_stats_field[@]};i++)); do
		end_stats[${bw_stats_field[i]}]=$(echo "$stats"|grep "${bw_stats_field[i]}"| awk '{print $2}')
	done
}

bw_start_stats(){
	for((i=0;i<${#bw_stats_field[@]};i++)); do
		start_stats[${bw_stats_field[i]}]=${end_stats[${bw_stats_field[i]}]}
	done
}

get_bw_fmt(){
	pre_period_bytes=$1
	cur_period_bytes=$2
	time_diff=$3

	if [ ! "$m" ];then
		fmt_factor=$((1000*1000*1000/8))
	else
		fmt_factor=$((1024*1024))
	fi

	bw_fmt=$(printf "%.2f" $(echo "scale=2;($cur_period_bytes-$pre_period_bytes)/$fmt_factor/$time_diff" | bc))
	echo "$bw_fmt"
}

print_stats(){
	device=$1
	time_interval=$2
	echo "$device roce"

	if [ ! "$m" ];then
		tx_print_fmt='tx_bw: %6.2f Gb/s\n'
		rx_print_fmt='rx_bw: %6.2f Gb/s\n'
	else
		tx_print_fmt='tx_bw: %8.2f MiB/s\n'
		rx_print_fmt='rx_bw: %8.2f MiB/s\n'
	fi

	for((i=0;i<${#bw_stats_field[@]};i++)); do
		bw_res=$(get_bw_fmt "${start_stats[${bw_stats_field[i]}]}" "${end_stats[${bw_stats_field[i]}]}" "$time_interval")
		if [ $i -eq 0 ];then
			printf "$tx_print_fmt" ${bw_res}
		else
			printf "$rx_print_fmt" ${bw_res}
		fi
	done
}

get_stats_info(){
	device=$1
	time_limit=$2
	init_start_stats "$device"
	t1=$(date +%s)

	while true;do
		sleep "$time_limit"
		bw_end_stats "$device"
		t2=$(date +%s)
		duration=$((t2-t1))
		print_stats "$device" "$duration"
		t1=$t2
		bw_start_stats
	done
}

help_info="
Usage: $0 [OPTIONS]

Show real-time bandwidth

Example:
	$ sh get_bw.sh -d eth2 -t 1
	eth2 roce
	tx_bw:   6.07 Gb/s
	rx_bw:  98.64 Gb/s

	$ sh get_bw.sh -d eth2 -t 1 -m
	eth2 roce
	tx_bw:   732.07 MiB/s
	rx_bw: 11889.87 MiB/s

Options:
  -h, --help		show this info
  -d, --device		device name
  -t, --time		time interval of measuring bandwidth
  -m, --report_MiB	Report BW of test in MiB/sec (instead of Gbit/sec)
"

while [[ $# -gt 0 ]]; do
	case $1 in
	-h|--help)
		echo "$help_info"
		exit 0
		;;
	-d|--device)
		d=$2
		shift
		;;
	-t|--time)
		t=$2
		shift
		;;
	-m|--report_MiB)
		m=1
		;;
	*)
		echo "Invalid option:$1"
		echo "$help_info"
		exit 0
		;;
	esac
	shift
done

if [ ! "$d" ]; then
	echo "device name is not set"
	echo "$help_info"
	exit 0
elif [ ! "$t" ]; then
	echo "Interval Time is not set"
	echo "$help_info"
	exit 0
fi

get_stats_info "$d" "$t"
