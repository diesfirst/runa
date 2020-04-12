import subprocess
import os

BASEDIR = os.path.join( os.path.dirname(__file__), '..')
SHADER_DIRS = [BASEDIR + '/src/shaders/basic/', BASEDIR + '/src/shaders/tarot/']
OUTDIR = BASEDIR + '/build/shaders/'
GLSLC_PATH = '/home/michaelb/dev/Vulkan/1.1.126.0/x86_64/bin/glslc'

def compileShaders(shader_dir):
    shaders = []
    files = os.listdir(shader_dir)
    for f in files:
        ext = str.split(f, '.')[-1]
        name = str.split(f, '.')[0]
        if ext == 'frag' or ext == 'vert':
            shaders.append((shader_dir+f, OUTDIR + name + '.spv'))
            
    print(shaders)
    for shader in shaders:
        subprocess.run([GLSLC_PATH, shader[0], '-o', shader[1]])

for directory in SHADER_DIRS:
    compileShaders(directory)
