#!/usr/local/bin/python

for code in xrange (ord ('a'), ord('z')+1):
    print ('register_scancode (L, "' + chr(code) + '", SDL_SCANCODE_' + chr(code).upper() + ');')
for code in xrange (ord ('0'), ord('9')+1):
    print ('register_scancode (L, "' + chr(code) + '", SDL_SCANCODE_' + chr(code) + ');')
