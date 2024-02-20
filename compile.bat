glslc shaders/shader.vert -o shaders/vert.spv
glslc shaders/shader.frag -o shaders/frag.spv
glslc shaders/spectrum.comp -o shaders/spectrum.spv
glslc shaders/butterfly.comp -o shaders/butterfly.spv
cmake --build build -j 8
