
glslc.exe verticesOnly.vert -o compiledShaders/verticesOnlyVert.spv
glslc.exe verticesOnly.frag -o compiledShaders/verticesOnlyFrag.spv
glslc.exe lit.vert -o compiledShaders/litVert.spv
glslc.exe lit.frag -o compiledShaders/litFrag.spv
glslc.exe unlit.vert -o compiledShaders/unlitVert.spv
glslc.exe unlit.frag -o compiledShaders/unlitFrag.spv
glslc.exe pbr.vert -o compiledShaders/pbrVert.spv
glslc.exe pbr.frag -o compiledShaders/pbrFrag.spv
glslc.exe bump.vert -o compiledShaders/bumpVert.spv
glslc.exe bump.frag -o compiledShaders/bumpFrag.spv
glslc.exe water.vert -o compiledShaders/waterVert.spv
glslc.exe water.frag -o compiledShaders/waterFrag.spv
glslc.exe textureWarp.vert -o compiledShaders/textureWarpVert.spv
glslc.exe textureWarp.frag -o compiledShaders/textureWarpFrag.spv
glslc.exe ubo.vert -o compiledShaders/uboVert.spv
glslc.exe ubo.frag -o compiledShaders/uboFrag.spv


pause