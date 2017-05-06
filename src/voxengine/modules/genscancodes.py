#!/usr/local/bin/python

for code in xrange (ord ('a'), ord('z')+1):
    print ('register_scancode (L, "' + chr(code) + '", SDL_SCANCODE_' + chr(code).upper() + ');')
for code in xrange (ord ('0'), ord('9')+1):
    print ('register_scancode (L, "' + chr(code) + '", SDL_SCANCODE_' + chr(code) + ');')
for code in xrange (1, 13):
    print ('register_scancode (L, "F' + str(code) + '", SDL_SCANCODE_F' + str(code) + ');')
