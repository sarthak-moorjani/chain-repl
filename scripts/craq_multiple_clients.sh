#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Error: Expects config file with server information"
  exit 1
fi

PORT=50060
CLIENTSERVER=
while IFS= read -r line; do
				SERVERID=$(echo $line | cut -d "," -f 1)
				if [[ "$SERVERID" == 1 ]]; then
								CLIENTSERVER=$(echo $line | cut -d "," -f 2 | cut -d ":" -f 1)
				fi
done < "$1"

CLIENTPORT="$CLIENTSERVER${PORT}"
CONFIGFILE="$1"
USER="$2"

# install gnu-parallel
#sudo apt-get update
yum update
#sudo apt-get install parallel

clients=1

# change working directory from scripts
pushd ../

echo $(pwd)

# Compile
rm -rf src/craq/cmake/build
mkdir -p src/craq/cmake/build
pushd src/craq/cmake/build
cmake ../..
make -j 4
popd

echo $(pwd)

export SHELL=$(type -p bash)
export CLIENTPORT=$CLIENTPORT
export CONFIGFILE=$CONFIGFILE
export USER=$USER

run_cpp_executable() {
    input_file=$1
    output_file="${input_file%.*}_output.txt"
    echo "Processing ${input_file} -> ${output_file}"
    echo $CLIENTPORT
    ./chain_client $CLIENTPORT ${input_file} $CONFIGFILE $USER
}

export -f run_cpp_executable

echo $(pwd)
# List of read_workload nput files to process
pushd inputs/write_workload
input_files=($(ls -1t ./write_workload_input*.txt | head -n $clients))
echo "${input_files[@]}"
popd

echo $(pwd)

echo $input_files
# exit
#  Run gnu-parallel
pushd ./src/craq/cmake/build

echo $(pwd)
COUNTER=1
for input_file in "${input_files[@]}"; do
				if [ $COUNTER -lt $clients ]; then
								CLIENTPORT="${CLIENTSERVER}:${PORT}"
								echo "$CLIENTPORT"
								./chain_client $CLIENTPORT $input_file $CONFIGFILE $USER > $COUNTER.out &
				fi
				COUNTER=$(expr $COUNTER + 1)
				PORT=$(expr $PORT + 1)
done

CLIENTPORT="${CLIENTSERVER}:${PORT}"
echo "$CLIENTPORT"
./chain_client $CLIENTPORT ${input_files[-1]} $CONFIGFILE $USER

#parallel run_cpp_executable {} ::: "1" "${input_files[@]}"

