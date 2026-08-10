#!/usr/bin/env bash
# Build RuView ESP32-C6 CSI firmware from source and merge a single factory.bin.
# Run INSIDE the espressif/idf:v5.4 container, with this dir mounted at /project.
set -e
cd /project

echo "[1/3] set-target esp32c6 (applies sdkconfig.defaults.esp32c6 -> 4MB partitions)"
idf.py set-target esp32c6

echo "[2/3] build"
idf.py build

echo "[3/3] merge factory.bin"
# partitions_4mb.csv layout: pt @0x8000, nvs @0x9000, otadata @0xF000, app(ota_0) @0x20000
esptool.py --chip esp32c6 merge_bin \
  --flash_size 4MB \
  --output factory.bin \
  0x0      build/bootloader/bootloader.bin \
  0x8000   build/partition_table/partition-table.bin \
  0xF000   build/ota_data_initial.bin \
  0x20000  build/esp32-csi-node.bin

echo "[done] factory.bin:"
ls -la factory.bin
esptool.py --chip esp32c6 image_info factory.bin 2>&1 | head -20 || true
