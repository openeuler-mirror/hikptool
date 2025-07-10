#!/bin/bash

# The debugfs of the driver can be used to dump the binary data of the NCL file
# used by the current network port (16K binary data). The first 1k (0-0x3FF)
# is the NCL header, indicating the NCL configuration information of the current
# board.The data is used to decompress ncl_config during imp initialization.

# The data dumped by using the debugfs file is a single NCL_CONFIG file after
# decompression. The actual size of a single valid NCL is 8K. Therefore, we can directly
# view 8K data between 0x400 and 0x2400.

# the column for data
DATA_COL=2

# The top 1K and last 7K is not used for nic/roce
BASE_OFFSET="0x0400"
HIGH_OFFSET="0x2400"

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
	if [ -e /sys/class/infiniband/"$name" ]; then
		ethname=$(ls /sys/class/infiniband/"$name"/device/net/ 2>/dev/null)
	fi

	# support net devname
	# get BDF from net devname
	BDF=$(ethtool -i "$ethname" |grep "bus-info" | awk '{print $2}')
	echo "$BDF"
	check_whether_hns3 "$ethname"
	return $?
}

big_endian_2_little_endian() {
	# Output the characters corresponding to each byte in little-endian order
	data=$1
	byte1=$((data & 0xff))
	byte2=$((data >> 8 & 0xff))
	byte3=$((data >> 16 & 0xff))
	byte4=$((data >> 24 & 0xff))
	echo -ne "$(printf '\\x%02x\\x%02x\\x%02x\\x%02x' $byte1 $byte2 $byte3 $byte4)"
}

generate_ncl() {
	tempfile=$(mktemp)
	rawfile=/sys/kernel/debug/hns3/"$BDF"/ncl_config
	# Save NCL in a tempfile, Only 8K data that is useful in the middle is retained
	BASE_OFFSET_LINE=$(grep -nr "${BASE_OFFSET} |" "$rawfile" | awk '{print $1}' | cut -d ':' -f 1)
	HIGH_OFFSET_LINE=$(grep -nr "${HIGH_OFFSET} |" "$rawfile" | awk '{print $1}' | cut -d ':' -f 1)
	awk -v base="$BASE_OFFSET_LINE" -v high="$HIGH_OFFSET_LINE" 'NR>=base && NR<high { print }' < "$rawfile" > "$tempfile"
	#read tempfile
	while read -r line; do
		data=$(echo "$line" | cut -d '|' -f $DATA_COL)
		big_endian_2_little_endian "${data// /}"
	done < "$tempfile"

	rm "$tempfile"
}

help_info="
Usage: $0 [OPTIONS]

Get ncl_config, Only support hns.
please use ncl_config_opt.xlsm to parse ncl_config
Use the NCL0 tab to read a single NCL

Options:
  -h, --help		show this info
  -v, --version		version info
  -d, --device		device name, support netdev name, rdmadev name and BDF
"

VERSION="1.0"
while [[ $# -gt 0 ]]; do
	case $1 in
	-h|--help)
		echo "$help_info"
		exit 0
		;;
	-v|--version)
		echo "Version $VERSION"
		exit 0
		;;
	-d|--device)
		dev=$2
		shift
		;;
	*)
		echo "Error: Invalid option: $1"
		echo "$help_info"
		exit 0
		;;
	esac
	shift
done

if [ ! "$dev" ]; then
	echo "Error: device name is not set"
	echo "$help_info"
	exit 0
fi

if ! BDF=$(get_bdf "$dev"); then
	echo "$help_info"
	exit 0
fi

generate_ncl
