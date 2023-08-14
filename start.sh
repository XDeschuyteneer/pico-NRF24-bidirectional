#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BIN="${SCRIPT_DIR}/build/Main.uf2"


mkdir -p ${SCRIPT_DIR}/build
cd ${SCRIPT_DIR}/build
cmake ..
make

PICOS=$(lsusb | grep Pico | grep Serial:)
BUSES=$(echo "${PICOS}" | awk '{printf("%d\n",$2)}' | uniq)
DEVICES=$(echo "${PICOS}" | awk '{printf("%d\n",$4)}' | uniq)

for bus in ${BUSES}; do
    for address in ${DEVICES}; do
        echo "Installing to bus ${bus} address ${address}"
        picotool load ${BIN} --bus ${bus} --address ${address} -f
    done
done
