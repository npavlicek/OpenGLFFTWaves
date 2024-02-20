import glob
import subprocess
import ntpath
import pathlib
import sys

"""
This file is configured to use glslc to compile shaders.
It also uses cmake to build our application.
"""

def compile_shaders(type, shaders):
	print(f'Compiling {type} shaders...')
	for s in shaders:
		filename = ntpath.basename(s).split('.')[0]
		command = f'glslc {s} -o .\\shaders\\compiled\\{type}\\{filename}.spv'
		print(f'Compiling: {s}')
		output = subprocess.run(command, capture_output=True)
		if output.returncode != 0:
			print('ERROR: ' + output.stderr.decode())
	print()

pathlib.Path()

# Create missing directories
pathlib.Path.mkdir(pathlib.Path(r'.\shaders\compiled\vertex'), parents=True, exist_ok=True)
pathlib.Path.mkdir(pathlib.Path(r'.\shaders\compiled\fragment'), parents=True, exist_ok=True)
pathlib.Path.mkdir(pathlib.Path(r'.\shaders\compiled\compute'), parents=True, exist_ok=True)

# Get list of shaders
compute_shaders = glob.glob(r".\shaders\*.comp")
vertex_shaders = glob.glob(r".\shaders\*.vert")
fragment_shaders = glob.glob(r".\shaders\*.frag")

# Compile the shaders
compile_shaders('compute', compute_shaders)
compile_shaders('vertex', vertex_shaders)
compile_shaders('fragment', fragment_shaders)

# Finally compile the app
print('Compiling application...')
output = subprocess.Popen('cmake --build build -j 8', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
while output.poll() is None:
	l = output.stdout.readline()
	sys.stdout.write(l.decode())
sys.stdout.write(output.stdout.read().decode())

# Check for errors
if (output.returncode != 0):
	sys.stdout.write(output.stderr.read().decode())
