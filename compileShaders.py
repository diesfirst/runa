import subprocess
import os

SHADER_DIR = 'shaders/'
GLSLC_PATH = '/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/bin/glslc'

files = os.listdir(SHADER_DIR)

shaders = []
for f in files:
    ext = str.split(f, '.')[-1]
    name = str.split(f, '.')[0]
    if ext == 'frag' or ext == 'vert':
        shaders.append((SHADER_DIR+f, SHADER_DIR + name + '.spv'))
        
print(shaders)
for shader in shaders:
    subprocess.run([GLSLC_PATH, shader[0], '-o', shader[1]])
