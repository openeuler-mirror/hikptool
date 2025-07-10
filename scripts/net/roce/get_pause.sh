#!/bin/bash
INPUT_NAME=$1

tx_pause_stats=("tx_mac_pause" "tx_pause_xoff_time")
rx_pause_stats=("rx_mac_pause" "rx_pause_xoff_time")
declare -a rx_pfc_stats
declare -a tx_pfc_stats
declare -A start_stats
declare -A end_stats

init_pfc_array() {
	tx_pfc_stats+=("tx_pfc_pkt")
	rx_pfc_stats+=("rx_pfc_pkt")
	for((i=0; i<8; i++)); do
		rx_pfc_stats+=("rx_pfc_pri${i}_pkt")
		rx_pfc_stats+=("rx_pfc_pri${i}_xoff_time")
		tx_pfc_stats+=("tx_pfc_pri${i}_pkt")
		tx_pfc_stats+=("tx_pfc_pri${i}_xoff_time")
	done
}

invalid_netdev() {
	netdev=$1
	echo "$netdev"
	ethtool -i "$netdev" |grep hns3 &> /dev/null
	return $?
}

get_dev_name() {
	name=$1
	ethname=""
	# support rdma dev name
	if rdma_ret=$(rdma link show "$name/1" 2>/dev/null); then
		ethname=$(echo "$rdma_ret"|awk '{print $NF}')
		invalid_netdev "$ethname"
		return $?
	fi
	# support BDF
	BDF=$(lspci -s "$name" -nD 2>/dev/null | awk '{print $1}' 2>/dev/null)
	if ethname=$(ls /sys/bus/pci/devices/"$BDF"/net/ 2>/dev/null); then
		invalid_netdev "$ethname"
		return $?
	fi
	# support net devname
	invalid_netdev "$name"
	return $?
}

init_start_stats() {
	local device=$1

	stats=$(ethtool -S "$device")
	for ((i=0; i<${#tx_pause_stats[@]}; i++)); do
		start_stats[${tx_pause_stats[i]}]=$(echo "$stats"|grep "${tx_pause_stats[i]}"| awk '{print $2}')
		start_stats[${rx_pause_stats[i]}]=$(echo "$stats"|grep "${rx_pause_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#tx_pfc_stats[@]}; i++)); do
		start_stats[${tx_pfc_stats[i]}]=$(echo "$stats"|grep "${tx_pfc_stats[i]}"| awk '{print $2}')
		start_stats[${rx_pfc_stats[i]}]=$(echo "$stats"|grep "${rx_pfc_stats[i]}"| awk '{print $2}')
	done
}

fresh_end_stats() {
	local device=$1

	stats=$(ethtool -S "$device")
	for ((i=0; i<${#tx_pause_stats[@]}; i++)); do
		end_stats[${tx_pause_stats[i]}]=$(echo "$stats"|grep "${tx_pause_stats[i]}"| awk '{print $2}')
		end_stats[${rx_pause_stats[i]}]=$(echo "$stats"|grep "${rx_pause_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#tx_pfc_stats[@]}; i++)); do
		end_stats[${tx_pfc_stats[i]}]=$(echo "$stats"|grep "${tx_pfc_stats[i]}"| awk '{print $2}')
		end_stats[${rx_pfc_stats[i]}]=$(echo "$stats"|grep "${rx_pfc_stats[i]}"| awk '{print $2}')
	done
}

fresh_start_stats() {
	for ((i=0; i<${#tx_pause_stats[@]}; i++)); do
		start_stats[${tx_pause_stats[i]}]=${end_stats[${tx_pause_stats[i]}]}
		start_stats[${rx_pause_stats[i]}]=${end_stats[${rx_pause_stats[i]}]}
	done
	for ((i=0; i<${#tx_pfc_stats[@]}; i++)); do
		start_stats[${tx_pfc_stats[i]}]=${end_stats[${tx_pfc_stats[i]}]}
		start_stats[${rx_pfc_stats[i]}]=${end_stats[${rx_pfc_stats[i]}]}
	done
}

print_stats() {
	for ((i=0; i<${#tx_pause_stats[@]}; i++)); do
		rx_ret=$(( ${end_stats[${rx_pause_stats[i]}]} - ${start_stats[${rx_pause_stats[i]}]} ))
		tx_ret=$(( ${end_stats[${tx_pause_stats[i]}]} - ${start_stats[${tx_pause_stats[i]}]} ))
		if [ $rx_ret -ne 0 ] || [ $tx_ret -ne 0 ];then
			echo "${rx_pause_stats[i]}: $rx_ret, ${tx_pause_stats[i]}: $tx_ret"
		fi
	done
	for ((i=0; i<${#tx_pfc_stats[@]}; i++)); do
		rx_ret=$(( ${end_stats[${rx_pfc_stats[i]}]} - ${start_stats[${rx_pfc_stats[i]}]} ))
		tx_ret=$(( ${end_stats[${tx_pfc_stats[i]}]} - ${start_stats[${tx_pfc_stats[i]}]} ))
		if [ $rx_ret -ne 0 ] || [ $tx_ret -ne 0 ];then
			echo "${rx_pfc_stats[i]}: $rx_ret, ${tx_pfc_stats[i]}: $tx_ret"
		fi
	done
	echo "**************************************************"
}

get_period_pause_info() {
	device=$1
	init_pfc_array
	init_start_stats "$device"
	while true; do
		sleep 1
		fresh_end_stats "$device"
		print_stats
		fresh_start_stats
	done
}

help_info="
Usage: $0 [OPTIONS]

Get pause frame stats per second. Only support hns.

Options:
  -h, --help    show this info
  -v, --version version info
  dev,          device name, support netdev name, rdmadev name and BDF
"

while [[ $# -gt 0 ]]; do
	case $1 in
	-h|--help)
		echo "$help_info"
		exit 0
		;;
	-v|--version)
		echo "Version 1.0"
		exit 0
		;;
	*)
		if ! DEV=$(get_dev_name "$INPUT_NAME"); then
			echo "$help_info"
			exit 1
		fi
		get_period_pause_info "$DEV"
		;;
	esac
	shift
done
