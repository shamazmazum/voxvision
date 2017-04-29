vt = voxtrees
vr = voxrnd
vs = voxsdl

function init ()
   local test = function (sample) return sample > 40 end
   -- Build tree from a raw file 'skull.dat' in voxvision's system-wide data directory
   -- 'skull.dat' is provided as an example.
   local tree = vt.read_raw_data (vt.find_data_file "skull.dat", vt.dot (256,256,256), 1, test)
   print (#tree)

   local camera = vr.simple_camera()
   camera:set_position (vt.dot (100,60,-100))
   camera:set_rot_angles (vt.dot (0.7, 0, 0))
   return tree, camera
end

function tick (tree, camera, time, framedelta)
   -- Get keyboard state
   local keystate = vs.get_keyboard_state()
   --[[
      This is as in previous example. 'voxutils' table has a function process_movement
      to process movement in one line of code. It can be called so:
      process_movement (keystate, camera, mdelta, adelta, controls).
      mdelta is delta for move_camera method and adelta is delta for rotate_camera
      control keys can be redefined in 'controls' table (see source code)
   ]]--

   -- Also 'framedelta' contains time in milliseconds taken to render previous frame
   voxutils.process_movement (keystate, camera, 0.25*framedelta)
end
