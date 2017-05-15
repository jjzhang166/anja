#@  {
#@  "targets":
#@      [
#@           {
#@			 "name":"knob_ambient_in.png","dependencies":
#@				[{"ref":"knob_sprites.blend","rel":"file"},{"ref":"blender","rel":"tool"}]
#@			 }
#@           ,{
#@			 "name":"knob_diffuse_in.png","dependencies":
#@				[{"ref":"knob_sprites.blend","rel":"file"},{"ref":"blender","rel":"tool"}]
#@			 }
#@           ,{
#@			 "name":"knob_mask_in.png","dependencies":
#@				[{"ref":"knob_sprites.blend","rel":"file"},{"ref":"blender","rel":"tool"}]
#@			 }
#@      ]
#@  }

import subprocess
import sys
import os

def write_error(*args, **kwargs):
    print(*args,file=sys.stderr,**kwargs)

try:
	target_dir=sys.argv[1];
	in_dir=sys.argv[2];

	print('# Baking knob sprites')

	blender=subprocess.Popen(['blender','-b',in_dir+'/knob_sprites.blend' \
		,'-s','1','-e','3','-o',target_dir+'/'+in_dir+'/knob_#.png','-a'] \
		,stdout=subprocess.PIPE)
	for lines in blender.stdout:
		progress=lines.decode('utf8').rstrip().split('|')
		print('# Blender: %s'%(progress[-1].strip()))

	os.rename(target_dir+'/'+in_dir+'/knob_1.png',target_dir+'/'+in_dir+'/knob_ambient_in.png')
	os.rename(target_dir+'/'+in_dir+'/knob_2.png',target_dir+'/'+in_dir+'/knob_diffuse_in.png')
	os.rename(target_dir+'/'+in_dir+'/knob_3.png',target_dir+'/'+in_dir+'/knob_mask_in.png')
	sys.exit(0)
except Exception:
	write_error('%s:%d: error: %s\n'%(sys.argv[0],sys.exc_info()[2].tb_lineno,sys.exc_info()[1]))
	sys.exit(-1)
