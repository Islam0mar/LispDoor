target remote | openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -f gdb.cfg
monitor halt
monitor gdb_sync
stepi
