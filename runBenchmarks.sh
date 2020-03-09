# sudo cpupower frequency-set --governor performance
(make Benchmarks config=release && cd bin/linux_x86_64/Release/Benchmarks; ./Benchmarks --benchmark_out=bench.json --benchmark_out_format=json)
mv bin/linux_x86_64/Release/Benchmarks/bench.json BenchmarksResults/ecspp.json
# sudo cpupower frequency-set --governor powersave