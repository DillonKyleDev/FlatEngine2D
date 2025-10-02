
glslc.exe verticesOnly.vert -o compiledShaders/verticesOnlyVert.spv
glslc.exe verticesOnly.frag -o compiledShaders/verticesOnlyFrag.spv
glslc.exe lit.vert -o compiledShaders/litVert.spv
glslc.exe lit.frag -o compiledShaders/litFrag.spv
glslc.exe unlit.vert -o compiledShaders/unlitVert.spv
glslc.exe unlit.frag -o compiledShaders/unlitFrag.spv
glslc.exe strip.vert -o compiledShaders/stripVert.spv
glslc.exe strip.frag -o compiledShaders/stripFrag.spv
glslc.exe stripParticle.vert -o compiledShaders/stripParticleVert.spv
glslc.exe stripParticle.frag -o compiledShaders/stripParticleFrag.spv
glslc.exe water.vert -o compiledShaders/waterVert.spv
glslc.exe water.frag -o compiledShaders/waterFrag.spv
glslc.exe textureWarp.vert -o compiledShaders/textureWarpVert.spv
glslc.exe textureWarp.frag -o compiledShaders/textureWarpFrag.spv


pause