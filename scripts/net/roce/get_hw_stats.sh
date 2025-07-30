#!/bin/bash
rdma_tx_stats=("tx_pkt" "tx_err_pkt" "tx_cnp_pkt" "tx_rc_pkt" "tx_ud_pkt" "tx_xrc_pkt")
rdma_rx_stats=("rx_pkt" "rx_err_pkt" "rx_cnp_pkt" "rx_rc_pkt" "rx_ud_pkt" "rx_xrc_pkt")
rdma_cq_stats=("cq_cqe" "cq_poe" "cq_notify")
rdma_trp_stats=("get_mpt_err_pkt" "get_irrl_err_pkt")
ETH_STATS=false
eth_tx_stats=("mac_tx_total_pkt" "mac_tx_good_pkt" "mac_tx_bad_pkt" "mac_tx_err_all_pkt")
eth_rx_stats=("mac_rx_total_pkt" "mac_rx_good_pkt" "mac_rx_bad_pkt" "mac_rx_fcs_err_pkt")

declare -A start_stat
declare -A end_stat

invalid_netdev() {
	local netdev=$1
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

get_rdma_name() {
	ethname=$1
	rdma_name=$(ls /sys/class/net/"$ethname"/device/infiniband/ 2>/dev/null)
	echo "$rdma_name"
}

print_rdma_stats() {
	for ((i=0; i<${#rdma_rx_stats[@]}; i++)); do
		rx_ret=$(( ${end_stat[${rdma_rx_stats[i]}]} - ${start_stat[${rdma_rx_stats[i]}]} ))
		tx_ret=$(( ${end_stat[${rdma_tx_stats[i]}]} - ${start_stat[${rdma_tx_stats[i]}]} ))
		if [ $rx_ret -ne 0 ] || [ $tx_ret -ne 0 ];then
			echo "${rdma_rx_stats[i]}: $rx_ret, ${rdma_tx_stats[i]}: $tx_ret"
		fi
	done

	cq_str=""
	for ((i=0; i<${#rdma_cq_stats[@]}; i++)); do
		ret=$(( ${end_stat[${rdma_cq_stats[i]}]} - ${start_stat[${rdma_cq_stats[i]}]} ))
		if [ $ret -ne 0 ];then
			cq_str="${rdma_cq_stats[i]}: $ret;"" ""${cq_str}"
		fi
	done
	if [ -n "${cq_str}" ]; then
		echo "${cq_str}"
	fi

	trp_str=""
	for ((i=0; i<${#rdma_trp_stats[@]}; i++)); do
		ret=$(( ${end_stat[${rdma_trp_stats[i]}]} - ${start_stat[${rdma_trp_stats[i]}]} ))
		if [ $ret -ne 0 ];then
			cq_str="${rdma_trp_stats[i]}: $ret;"" ""${trp_str}"
		fi
	done
	if [ -n "${trp_str}" ]; then
		echo "${trp_str}"
	fi
}

fresh_rdma_start_stats() {
	for ((i=0; i<${#rdma_rx_stats[@]}; i++)); do
		start_stat[${rdma_tx_stats[i]}]=${end_stat[${rdma_tx_stats[i]}]}
		start_stat[${rdma_rx_stats[i]}]=${end_stat[${rdma_rx_stats[i]}]}
	done

	for ((i=0; i<${#rdma_cq_stats[@]}; i++)); do
		start_stat[${rdma_cq_stats[i]}]=${end_stat[${rdma_cq_stats[i]}]}
	done

	for ((i=0; i<${#rdma_trp_stats[@]}; i++)); do
		start_stat[${rdma_cq_stats[i]}]=${end_stat[${rdma_cq_stats[i]}]}
	done
}

init_rdma_stats() {
	local device=$1
	stats=$(rdma stat show link "$device" -p)
	for ((i=0; i<${#rdma_rx_stats[@]}; i++)); do
		start_stat[${rdma_rx_stats[i]}]=$(echo "$stats"|grep "${rdma_rx_stats[i]}"| awk '{print $2}')
		start_stat[${rdma_tx_stats[i]}]=$(echo "$stats"|grep "${rdma_tx_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#rdma_cq_stats[@]}; i++)); do
		start_stat[${rdma_cq_stats[i]}]=$(echo "$stats"|grep "${rdma_cq_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#rdma_trp_stats[@]}; i++)); do
		start_stat[${rdma_trp_stats[i]}]=$(echo "$stats"|grep "${rdma_trp_stats[i]}"| awk '{print $2}')
	done
}

fresh_rdma_end_stats() {
	stats=$1
	for ((i=0; i<${#rdma_rx_stats[@]}; i++)); do
		end_stat[${rdma_rx_stats[i]}]=$(echo "$stats"|grep "${rdma_rx_stats[i]}"| awk '{print $2}')
		end_stat[${rdma_tx_stats[i]}]=$(echo "$stats"|grep "${rdma_tx_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#rdma_cq_stats[@]}; i++)); do
		end_stat[${rdma_cq_stats[i]}]=$(echo "$stats"|grep "${rdma_cq_stats[i]}"| awk '{print $2}')
	done
	for ((i=0; i<${#rdma_trp_stats[@]}; i++)); do
		end_stat[${rdma_trp_stats[i]}]=$(echo "$stats"|grep "${rdma_trp_stats[i]}"| awk '{print $2}')
	done
}

init_eth_stats() {
	local device=$1
	stats=$(ethtool -S "$device")
	for ((i=0; i<${#eth_rx_stats[@]}; i++)); do
		start_stat[${eth_rx_stats[i]}]=$(echo "$stats"|grep "${eth_rx_stats[i]}"| awk '{print $2}')
		start_stat[${eth_tx_stats[i]}]=$(echo "$stats"|grep "${eth_tx_stats[i]}"| awk '{print $2}')
	done
}

fresh_eth_end_stats() {
	local stats=$1
	for ((i=0; i<${#eth_rx_stats[@]}; i++)); do
		end_stat[${eth_rx_stats[i]}]=$(echo "$stats"|grep "${eth_rx_stats[i]}"| awk '{print $2}')
		end_stat[${eth_tx_stats[i]}]=$(echo "$stats"|grep "${eth_tx_stats[i]}"| awk '{print $2}')
	done
}

fresh_eth_start_stats() {
	for ((i=0; i<${#eth_rx_stats[@]}; i++)); do
		start_stat[${eth_tx_stats[i]}]=${end_stat[${eth_tx_stats[i]}]}
		start_stat[${eth_rx_stats[i]}]=${end_stat[${eth_rx_stats[i]}]}
	done
}

print_eth_stats() {
	for ((i=0; i<${#eth_rx_stats[@]}; i++)); do
		rx_ret=$(( ${end_stat[${eth_rx_stats[i]}]} - ${start_stat[${eth_rx_stats[i]}]} ))
		tx_ret=$(( ${end_stat[${eth_tx_stats[i]}]} - ${start_stat[${eth_tx_stats[i]}]} ))
		if [ $rx_ret -ne 0 ] || [ $tx_ret -ne 0 ];then
			echo "${eth_rx_stats[i]}: $rx_ret, ${eth_tx_stats[i]}: $tx_ret"
		fi
	done
}

get_stats() {
	local device=""
	local eth_dev=""

	if [ $# -gt 1 ]; then
		eth_dev=$2
		init_eth_stats "$eth_dev"
	fi
	device=$1
	init_rdma_stats "${device}"
	while true; do
		sleep 1
		stats=$(rdma stat show link "$device" -p)
		if [ -n "${eth_dev}" ]; then
			eth_stats=$(ethtool -S "${eth_dev}")
			fresh_eth_end_stats "${eth_stats}"
			print_eth_stats
			fresh_eth_start_stats
		fi
		fresh_rdma_end_stats "${stats}"
		print_rdma_stats
		fresh_rdma_start_stats
		echo "************************************************"
	done
}

help_info="
Usage: $0 [Options] dev

Get RDMA related HW stats per second. Only support hns.

This script outputs RDMA packet statistics by default.

Args:
  dev,          device name, support netdev name, rdmadev name and BDF
Options:
  -h, --help    show this info
  -v, --version version info
  -a, --all     add Mac stat
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
	-a|--all)
		ETH_STATS=true
		;;
	*)
		INPUT_NAME=$1
		if ! ETHDEV=$(get_dev_name "$INPUT_NAME"); then
			echo "$help_info"
			exit 1
		fi
		RDMADEV=$(get_rdma_name "$ETHDEV")

		if ${ETH_STATS}; then
			get_stats "$RDMADEV" "$ETHDEV"
		else
			get_stats "$RDMADEV"
		fi
		;;
	esac
	shift
done
