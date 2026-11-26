pio pkg exec -p "tool-esptoolpy" -- esptool.py --chip ESP32 merge_bin -o merged-flash.bin `
    --flash_mode dio --flash_size 4MB `
     0x00001000 .pio\build\esp32dev\bootloader.bin `
     0x00008000 .pio\build\esp32dev\partitions.bin `
     0x0000e000 .pio\build\esp32dev\firmware.bin        