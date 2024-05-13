#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_newlines() {
    num_newlines=$1
    for ((i=1; i<=$num_newlines; i++))
    do
        echo ""
    done
}

declare -A algorithm_args_map # used to hold the algorithms names (dir names) and their program arguments
declare -a n_procs

algorithm_args_map["char_freq"]="file.txt"


root_path=$(pwd)
for algorithm in "${!algorithm_args_map[@]}"; do
    echo "--------------------------------"

    cd "${root_path}/${algorithm}"
    executable="${algorithm}_mpi"
    c_file="${executable}.c"

    # compile mpi program
    mpicc -o "${executable}" "${c_file}"

    # read program arguments (separated by spaces)
    read -r -a arg_array <<< "${algorithm_args_map[${algorithm}]}"

    n_procs=(1 2 4 6 8 10)
    for n_proc in "${n_procs[@]}"; do
        echo -e "[${BLUE}INFO${NC}]: Running ${c_file} with ${n_proc} processes."
        time mpiexec --oversubscribe -n "${n_proc}" "${executable}" "${arg_array[@]}"
        print_newlines 2
    done

    echo "--------------------------------"
done
