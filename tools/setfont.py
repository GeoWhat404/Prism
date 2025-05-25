#!/usr/bin/python3

import os
import sys

FONT_ASM_PATH = 'kernel/src/graphics/font/font.asm'
FONT_PATH = 'font'

def gen_psf(bdf):
    psf = bdf.replace('.bdf', '.psf')
    os.system(f'bdf2psf --fb {bdf} /usr/share/bdf2psf/standard.equivalents /usr/share/bdf2psf/useful.set /usr/share/bdf2psf/fontsets/Uni2.512 512 {psf}')

    return psf

def strip_unicode(psf):
    os.system(f'psfstriptable {psf} kernel/{FONT_PATH}/{psf}')

def write_asm(fontname):
    lines =  []
    with open(FONT_ASM_PATH, 'r') as f:
        lines = f.readlines()

    lines[0] = f'%define FONT "{FONT_PATH}/{fontname}"\n'
    f.close()


    with open(FONT_ASM_PATH, 'w') as f:
        f.writelines(lines)

    os.remove(fontname)


def main(bdf):
    psf = gen_psf(bdf)
    strip_unicode(psf)

    write_asm(psf)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: setfont.py <bdf file>')
        exit(1)

    main(sys.argv[1])

    print('I recommend running "make clean" and then "make run"')
    
