import glob
import subprocess
from pathlib import Path
import sys

"""
This file is configured to use glslc to compile shaders.
It also uses cmake to build our application.
"""

do_compile_shaders = False

res = input("Compile shaders too? (y/n): ")

if (res == 'y' or res == 'Y'):
	do_compile_shaders = True

shader_types = [
	'compute',
	'vertex',
	'fragment',
	'tcs',
	'tes'
]

glslc_opts = '--target-env=opengl -x glsl'

def compile_shaders(type, shaders):
	print(f'Compiling {type} shaders...')
	for s in shaders:
		filename = Path(s).name.split('.')[0]
		command = f'glslc {s} {glslc_opts} -o .\\shaders\\compiled\\{type}\\{filename}.spv'
		print(command)
		output = subprocess.run(command, capture_output=True)
		if output.returncode != 0:
			print('ERROR: ' + output.stderr.decode())
	print()

if do_compile_shaders:
	# Create missing directories
	compiled_shader_dir = r'.\shaders\compiled'
	for s in shader_types:
		dir = Path(f"{compiled_shader_dir}\\{s}")
		if not dir.exists():
			print(f"{dir} does not exist. Creating directory...")
			Path.mkdir(dir, parents=True, exist_ok=True)
	print()

	# Get list of shaders
	compute_shaders = glob.glob(r".\shaders\compute\*.comp")
	vertex_shaders = glob.glob(r".\shaders\vertex\*.vert")
	fragment_shaders = glob.glob(r".\shaders\fragment\*.frag")
	tcs_shaders = glob.glob(r".\shaders\tessellation\*.tesc")
	tes_shaders = glob.glob(r".\shaders\tessellation\*.tese")


	# Compile the shaders
	compile_shaders(shader_types[0], compute_shaders)
	compile_shaders(shader_types[1], vertex_shaders)
	compile_shaders(shader_types[2], fragment_shaders)
	compile_shaders(shader_types[3], tcs_shaders)
	compile_shaders(shader_types[4], tes_shaders)

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
