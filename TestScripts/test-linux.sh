#!/bin/bash

# Folder structure for linux should look as follows
# - vulkan_samples
# - test-linux.sh
# - assets (folder)
# - shaders (folder)

samples=(hack_array_buffer hack_dynamic_uniform_buffer hack_dynamic_uniform_buffer_gpu hack_dynamic_uniform_buffer_gpu_scatter hack_dynamic_uniform_buffer_scatter hack_dynamic_uniform_buffer_up_multi hack_dynamic_uniform_buffer_up_scatter hack_emulated_uniform_buffer hack_emulated_uniform_structuredbuffer hack_multiple_uniform_buffer hack_multiple_uniform_buffer_descriptor_scratch hack_multiple_uniform_buffer_updatebuffer hack_offset_uniform_buffer hack_push_constant)

timestamp=`date "+%Y-%m-%d_%H-%M-%S"`

#create test results folder structure

TestResultDir="TestResults/$timestamp/SteamDeck"

mkdir -p $TestResultDir

for sample in ${samples[@]}
do
  /home/deck/.steam/steam/steamapps/common/SteamLinuxRuntime_sniper/run --terminal=tty -- ./vulkan_samples sample $sample
  if [ -f ./data.json ]; then
	  echo "Test result at: $TestResultDir/${sample}_data.json"
    mv ./data.json "./$TestResultDir/${sample}_data.json"
  else
    echo "Sample $sample did not produce a result"
  fi
done
