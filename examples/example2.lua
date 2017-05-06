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
   return {tree = tree, camera = camera}
end

function tick (world, time)
   -- Get keyboard state
   local keystate = vs.get_keyboard_state()
   local camera = world.camera

   -- If 's' key is pressed, move camera, and so on
   if keystate:keypressed (vs.scancodes.s) then
      camera:move_camera (vt.dot (0,-5,0))
   elseif keystate:keypressed (vs.scancodes.w) then
      camera:move_camera (vt.dot (0,5,0))
   end

   if keystate:keypressed (vs.scancodes.a) then
      camera:move_camera (vt.dot (-5,0,0))
   elseif keystate:keypressed (vs.scancodes.d) then
      camera:move_camera (vt.dot (5,0,0))
   end

   if keystate:keypressed (vs.scancodes.leftarrow) then
      camera:rotate_camera (vt.dot (0,0,0.05))
   elseif keystate:keypressed (vs.scancodes.rightarrow) then
      camera:rotate_camera (vt.dot (0,0,-0.05))
   end
   
   if keystate:keypressed (vs.scancodes.uparrow) then
      camera:rotate_camera (vt.dot (-0.05,0,0))
   elseif keystate:keypressed (vs.scancodes.downarrow) then
      camera:rotate_camera (vt.dot (0.05,0,0))
   end
end
