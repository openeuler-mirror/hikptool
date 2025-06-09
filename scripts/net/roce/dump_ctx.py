#!/usr/bin/env python3

"""
dump filtered rdma resource context by rdmatool
Only QP, CQ, MR, SRQ supported for now.
"""

import multiprocessing
import subprocess
import argparse
import json
import sys
import os

# Define resource condition_filters
class Filter:
    def __init__(self, name, is_number):
        self.name = name
        self.is_number = is_number

    def is_valid_filter_value(self, value):
        if self.is_number is True:
            return all(item.isdigit() for item in str(value).split("-"))

        return isinstance(value, str)

    def __str__(self):
        return f"filter: {self.name} is_number: {self.is_number}\n"

mr_condition_filters = {
    "dev": Filter("dev", False),
    "rkey": Filter("rkey", True),
    "lkey": Filter("lkey", True),
    "mrlen": Filter("mrlen", True),
    "pid": Filter("pid", True),
    "mrn": Filter("mrn", True),
    "pdn": Filter("pdn", True),
}

cq_condition_filters = {
    "dev": Filter("dev", False),
    "users": Filter("users", True),
    "poll-ctx": Filter("poll-ctx", False),
    "pid": Filter("pid", True),
    "cqn": Filter("cqn", True),
    "ctxn": Filter("ctxn", True),
}

qp_condition_filters = {
    "link": Filter("link", False),
    "lqpn": Filter("lqpn", True),
    "rqpn": Filter("rqpn", True),
    "pid": Filter("pid", True),
    "sq-psn": Filter("sq-psn", True),
    "rq-psn": Filter("rq-psn", True),
    "type": Filter("type", False),
    "path-mig-state": Filter("path-mig-state", False),
    "state": Filter("state", False),
    "pdn": Filter("pdn", True),
}

srq_condition_filters = {
    "dev": Filter("dev", False),
    "pid": Filter("pid", True),
    "srqn": Filter("srqn", True),
    "type": Filter("type", False),
    "pdn": Filter("pdn", True),
    "cqn": Filter("cqn", True),
    "lqpn": Filter("lqpn", True),
}

# Define the mapping relationship between resources to indices
res_to_idx_mapping = {
    'mr': 'mrn',
    'cq': 'cqn',
    'qp': 'lqpn',
    'srq': 'srqn',
}

# Define the mapping relationship between resources to driver indices
res_to_drv_idx_mapping = {
    'mr': 'mrn',
    'cq': 'drv_cqn',
    'qp': 'lqpn',
    'srq': 'drv_srqn',
}

# Define the mapping relationship between resources to contexts
res_to_ctx_mapping = {
    'mr': 'mpt',
    'cq': 'cqc',
    'qp': 'qpc',
    'srq': 'srqc',
}

# Define the mapping relationship between resources to condition filters
res_to_condition_filters_mapping = {
    'mr': mr_condition_filters,
    'cq': cq_condition_filters,
    'qp': qp_condition_filters,
    'srq': srq_condition_filters,
}

class CustomHelpFormatter(argparse.RawDescriptionHelpFormatter):
    def format_help(self):
        custom_help = super().format_help()
        custom_help += '\nSupport filter args for -c:\n'
        custom_help += '---------\n'
        for res, condition_filters in res_to_condition_filters_mapping.items():
            custom_help += f"{res} filters:\n"
            for _, condition_filter in condition_filters.items():
                custom_help += str(condition_filter)

        return custom_help

def args_define():
    # Create the argument parser
    parser = argparse.ArgumentParser(
        description="""
Dump filtered rdma resource context json.
""",
        formatter_class=CustomHelpFormatter,
        add_help=True,
        usage="""
dump_ctx [-h] -r {mr,cq,qp,srq} -d DEVICE [-c filter1 value1 ...] [-o OUTPUT_DIR] [-j JOBS]
""",
        epilog="""
Examples:
---------
Dump filtered QP resource context json:
    dump_ctx -r qp -d hns_4 -j 64 -c lqpn 22
    dump_ctx -r qp -d hns_4 -j 64 -c lqpn 1-1000
    dump_ctx -r qp -d hns_4 -j 64 -c type RC
    dump_ctx -r qp -d hns_4 -j 64 -c pid 16734 pdn 782
Dump filtered CQ resource context json:
    dump_ctx -r cq -d hns_4 -j 64 -c cqn 22
Dump filtered MR resource context json:
    dump_ctx -r mr -d hns_4 -j 64 -c mrn 22
Dump filtered SRQ resource context json:
    dump_ctx -r srq -d hns_4 -j 64 -c srqn 22
""")

    # Add arguments
    parser.add_argument(
        "-r", "--resource", choices=['mr', 'cq', 'qp', 'srq'], required=True,
        help="Specify an RDMA resource")
    parser.add_argument(
        "-d", "--device", type=str, required=True,
        help="Specify an RDMA device.")
    parser.add_argument(
        "-c", "--condition", type=str, nargs='*',
        help="""
Specify the RDMA resource context filter condition.
Multiple conditions can be specified. no more than 4 inputs.
""")
    parser.add_argument(
        "-o", "--output_dir", type=str,
        help="RDMA resource context archive dir")
    parser.add_argument(
        "-j", "--jobs", type=int, default=64,
        help="Allow N jobs at once.")

    return parser

def execute_command(command):
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as called_process_error:
        print(f"rdmatool command is incorrect: {called_process_error.stderr}")
        raise
    except Exception as exception:
        print(f"An error occurred: {exception}")
        raise

    return result.stdout

def get_rdma_res_json(res, dev, condition, is_raw = False):
    # Build the command
    command = ["rdma", "res", "show", res]

    if res == "qp":
        command.extend(["link", f"{dev}/1"])
    else:
        command.extend(["dev", f"{dev}"])

    if condition:
        command.extend(condition)

    if is_raw is True:
        command.append("-jpr")
    else:
        command.extend(["-j", "-dd"])

    rdma_res_str = execute_command(command)

    return json.loads(rdma_res_str)

def get_rdma_ctx_and_write_file(res, dev, condition, file_path):
    # Get the raw json data of rdma resources
    rdma_res_json_raw = get_rdma_res_json(res, dev, condition, is_raw = True)
    # Write to the file
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(json.dumps(rdma_res_json_raw, ensure_ascii=False))

def dump_filtered_rdma_res_ctx(inner_args):
    # Get the command parameters
    res = inner_args.resource
    dev = inner_args.device
    condition = inner_args.condition
    output_dir = inner_args.output_dir
    jobs = inner_args.jobs

    # If the user does not pass in output_dir, use the current directory
    output_dir = output_dir if output_dir else os.getcwd()

    # Get the json data of rdma resources
    rdma_res_json = get_rdma_res_json(res, dev, condition)

    # Get the filtered resource index dict
    res_idx_to_drv_idx_mapping = {
        item[res_to_idx_mapping[res]]: item[res_to_drv_idx_mapping[res]]
        for item in rdma_res_json
    }

    # Create directory
    directory_path = os.path.join(output_dir, 'rdma_res_ctx', res_to_ctx_mapping[res])
    os.makedirs(directory_path, exist_ok=True)

    # Multithreading query rdma_res_json_raw and write to directory_path
    pool = multiprocessing.Pool(processes = jobs)
    for res_idx, res_drv_idx in res_idx_to_drv_idx_mapping.items():
        # Build the file path
        file_path = os.path.join(directory_path, f"{res_to_ctx_mapping[res]}_{res_drv_idx}.log")
        condition = [res_to_idx_mapping[res], str(res_idx)]
        pool.apply_async(get_rdma_ctx_and_write_file,
                         args = (res, dev, condition, file_path))
    # Wait for all processes to complete
    pool.close()
    pool.join()

    print(f"Dump all rdma res ctx succ! Archived in {directory_path}")

def check_condition(condition):
    if condition is None:
        return

    # Check conditon arguments length
    if len(condition) > 4:
        raise ValueError("-c: no more than 4 inputs")

    # Check whether conditon arguments exists in the trustlist
    res_condition_filters = res_to_condition_filters_mapping[args.resource]
    condition_pair_list = list(zip(condition[::2], condition[1::2]))
    for filter_name, filter_value in condition_pair_list:
        if filter_name not in res_condition_filters:
            raise ValueError("-c: condition error detected")

        if res_condition_filters[filter_name].is_valid_filter_value(filter_value) is False:
            raise ValueError("-c: condition error detected")

if __name__ == "__main__":
    # Define the command line arguments
    arg_parser = args_define()

    try:
        import argcomplete
        argcomplete.autocomplete(arg_parser)
    except ImportError:
        pass

    # Parse the command line arguments
    args = arg_parser.parse_args()

    # Check condition arguments
    try:
        check_condition(args.condition)
    except ValueError as e:
        print(f"Error: {e}")
        arg_parser.print_usage()
        sys.exit(1)

    # Export the filtered rdma resource context
    dump_filtered_rdma_res_ctx(args)
