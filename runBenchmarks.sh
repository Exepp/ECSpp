# sudo cpupower frequency-set --governor performance
(make Benchmarks config=release && cd bin/linux_x86_64/Release/Benchmarks; ./Benchmarks)
# sudo cpupower frequency-set --governor powersave