#!/usr/bin/env python3
"""Build doomgeneric with the headless MK1 backend; no SDL or audio dependency."""
from pathlib import Path
import re, subprocess
root=Path(__file__).resolve().parents[1]
src=root/'third_party/doomgeneric/doomgeneric'
make=(src/'Makefile').read_text()
objects=re.search(r'^SRC_DOOM = (.+)$',make,re.M)[1].split()
files=[str(src/(o[:-2]+'.c')) for o in objects if o!='doomgeneric_xlib.o']
out=root/'build/Debug/mk1-freedoom';out.parent.mkdir(parents=True,exist_ok=True)
subprocess.run(['clang','-O2','-arch','arm64','-mmacosx-version-min=14.0','-DNORMALUNIX','-D_DEFAULT_SOURCE',
 '-DDOOMGENERIC_RESX=320','-DDOOMGENERIC_RESY=200',
 '-Wno-incompatible-pointer-types','-Wno-int-conversion',
 '-I'+str(src),'-I'+str(root/'mk1-freedoom'),*files,
 str(root/'mk1-freedoom/doomgeneric_mk1.c'),'-o',str(out)],check=True)
print(out)
