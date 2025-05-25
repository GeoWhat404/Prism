#!/bin/bash

# Tool to convert a BDF file into a non-unicode PSF binary that can be read
# param 1: input bdf
# param 2: output psf

set -xe

bdf2psf --fb $1 /usr/share/bdf2psf/standard.equivalents /usr/share/bdf2psf/fontsets/Uni2.512 512 $1.psf
psfstriptable $1.psf $2
rm $1.psf
