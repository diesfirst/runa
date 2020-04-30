import subprocess
import os

base_dir = os.path.join( os.path.dirname(__file__), '..')
shaders_dir = os.path.join(base_dir, "src", "shaders")
out_dir = os.path.join(base_dir, "build", "shaders")
glslc_path = '/home/michaelb/dev/Vulkan/1.1.126.0/x86_64/bin/glslc'


def compileShaders(shader_dir, out_dir, glslc_path):
    shaders = []
    files = os.listdir(shader_dir)
    for f in files:
        ext = str.split(f, '.')[-1]
        name = str.split(f, '.')[0]
        if ext == 'frag' or ext == 'vert':
#            shaders.append((shader_dir+f, out_dir + name + '.spv'))
            shaders.append((os.path.join(shader_dir, f), 
                os.path.join(out_dir, name + '.spv')))
            
    for shader in shaders:
        subprocess.run([glslc_path, shader[0], '-o', shader[1]])

dirs = [d[0] for d in os.walk(shaders_dir)]
for d in dirs:
    compileShaders(d, out_dir, glslc_path)
