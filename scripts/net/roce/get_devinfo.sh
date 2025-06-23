#!/bin/bash

INPUT_NAME=$1
# the column for data
data_col=2
tempfile=''

#The top 1K is not used for roce
BASE_OFFSET=$((16#400))
#The offset between various PFs in the roce general configuration
PF_OFFSET=$((16#48))

check_whether_hns3() {
	netdev=$1
	ethtool -i "$netdev" |grep hns3 &> /dev/null
	return $?
}

get_bdf() {
	name=$1
	# support BDF
	BDF=$(lspci -s "$name" -nD 2>/dev/null | awk '{print $1}' 2>/dev/null)
	if ethname=$(ls /sys/bus/pci/devices/"$BDF"/net/ 2>/dev/null); then
		echo "$BDF"
		check_whether_hns3 "$ethname"
		return $?
	fi

	ethname="$name"
	# support rdma dev name
	if rdma_ret=$(rdma link show "$name/1" 2>/dev/null); then
		ethname=$(echo "$rdma_ret"|awk '{print $NF}')
	fi

	# support net devname
	# get BDF from net devname
	BDF=$(ethtool -i "$ethname" |grep "bus-info" | awk '{print $2}')
	echo "$BDF"
	check_whether_hns3 "$ethname"
	return $?
}

get_pf_num() {
	echo "$1" |awk -F . '{printf $NF}'
}

# The current implementation logic performance is not good,
# it has to read an 8K file repeatedly. However, this tool
# has no performance requirements, so it can be implemented
# with the current simple solution.
get_data() {
	offset=$1
	bdf=$2
	#read the ncl
	while read -r line; do
		#get offset and data
		offset_hex=$(echo "$line" | cut -d '|' -f 1)
		data=$(echo "$line" | cut -d '|' -f $data_col)
		# get the data
		if [[ "${offset_hex// /}" == "$offset" ]]; then
			echo "$data"
			return 0
		fi
	done < "$tempfile"
}

get_cap_flag() {
	CAPFLAG_OFFSET=$((16#150))
	EXTCAP_OFFSET=$((16#16c))
	bdf=$1

	pf_num=$(get_pf_num "$bdf")
	pf_offset=$((PF_OFFSET * pf_num))

	offset=$((BASE_OFFSET + CAPFLAG_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" $offset)
	base_cap=$(get_data "$offset" "$bdf")
	base_cap=$((base_cap >> 20))

	offset=$((EXTCAP_OFFSET + BASE_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" "$offset")
	ext_cap=$(get_data "$offset" "$bdf")
	ext_cap=$((ext_cap >> 16))
	ext_cap=$((ext_cap << 12))

	cap=$(printf "0x%x" $((ext_cap | base_cap)))
	echo "$bdf capflags: $cap"
}

get_default_congest_type() {
	DEF_CONG_OFFSET=$((16#174))
	bdf=$1

	pf_num=$(get_pf_num "$bdf")
	pf_offset=$((PF_OFFSET * pf_num))

	offset=$((BASE_OFFSET + DEF_CONG_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" "$offset")
	def_cong=$(get_data "$offset" "$bdf")
	def_cong=$((def_cong >> 26))

	echo "$bdf defalut congest: $def_cong"
}

get_eq_info() {
	MAX_CEQ_OFFSET=$((16#170))
	MAX_AEQ_OFFSET=$((16#174))
	bdf=$1

	pf_num=$(get_pf_num "$bdf")
	pf_offset=$((PF_OFFSET * pf_num))

	offset=$((BASE_OFFSET + MAX_CEQ_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" "$offset")
	max_ceq_info=$(get_data "$offset" "$bdf")
	max_ceq=$((max_ceq_info >> 22))
	max_ceq_depth=$((max_ceq_info & 0x3fffff))
	max_ceq_depth=$(printf "max_ceq_depth: 0x%x" "$max_ceq_depth")

	offset=$((BASE_OFFSET + MAX_AEQ_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" "$offset")
	max_aeq_info=$(get_data "$offset" "$bdf")
	max_aeq_depth=$((max_aeq_info & 0x3fffff))
	max_aeq_depth=$(printf "max_aeq_depth: 0x%x" "$max_aeq_depth")

	echo "$bdf max_ceq: $max_ceq; $max_ceq_depth; $max_aeq_depth"
}

get_qp_hopnum() {
	QP_HOPNUM_OFFSET=$((16#168))
	bdf=$1

	pf_num=$(get_pf_num "$bdf")
	pf_offset=$((PF_OFFSET * pf_num))

	offset=$((BASE_OFFSET + QP_HOPNUM_OFFSET))
	offset=$((pf_offset + offset))
	offset=$(printf "0x%04x" "$offset")
	qp_hopnum=$(get_data "$offset" "$bdf")
	sq_hopnum=$((qp_hopnum >> 24))
	sq_hopnum=$((sq_hopnum & 0x3))
	sge_hopnum=$((qp_hopnum >> 22))
	sge_hopnum=$((sge_hopnum & 0x3))
	rq_hopnum=$((qp_hopnum >> 20))
	rq_hopnum=$((rq_hopnum & 0x3))

	echo "$bdf sq_hopnum: $sq_hopnum; sge_hopnum: $sge_hopnum; rq_hopnum: $rq_hopnum"
}

help_info="
Usage: $0 [OPTIONS]

Get device info from nclconfig, Only support hns.

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
		if ! DEV=$(get_bdf "$INPUT_NAME"); then
			echo "$help_info"
			exit 1
		fi
		tempfile=$(mktemp)
		# Save NCL in a tempfile to avoid accessing the IMP multiple times
		cat /sys/kernel/debug/hns3/"$DEV"/ncl_config > "$tempfile"
		get_cap_flag "$DEV"
		get_default_congest_type "$DEV"
		get_qp_hopnum "$DEV"
		get_eq_info "$DEV"
		rm "$tempfile"
		;;
	esac
	shift
done
