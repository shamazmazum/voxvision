vt = voxtrees
vr = voxrnd
vs = voxsdl

function init (ctx)
   -- Build tree from a raw file 'skull.dat' in voxvision's system-wide data directory
   -- 'skull.dat' is provided as an example.
   -- read_raw_data_ranged() is like faster read_raw_data() with test function checking
   -- min <= sample < max (in current example sample >= 40)
   print (ctx:get_geometry())
   local tree = vt.read_raw_data_ranged (vt.find_data_file "skull.dat", {256,256,256}, 1, 40)
   print (#tree)

   local camera = vr.camera "simple-camera"
   camera:set_position {100,60,-100}
   camera:set_rot_angles {1.4, 0, 0}

   -- Also create a collision detector
   local cd = vr.cd()
   -- Attach the context and your camera. 4 is the camera's body radius
   cd:attach_camera (camera, 4)
   cd:attach_context (ctx)

   -- Do not forget to add collision detector to the context to prevent GC'ing.
   ctx.tree = tree
   ctx.camera = camera
   ctx.cd = cd
   return true
end

function tick (world, time)
   local event, quit
   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown and event.keysym.sym == vs.key.Escape then
         quit = true
      elseif event.type == vs.event.Quit then
         quit = true
      end
   end
   if quit then return nil end

   -- Get keyboard state
   local keystate = vs.getKeyboardState()

   previous_time = previous_time or time
   local framedelta = time - previous_time
   --[[
      This is as in previous example. 'voxutils' table has a function
      process_keyboard_movement to process basic keyboard movement in one line
      of code. It can be called so:
      process_keyboard_movement (context, keystate, mdelta, controls).
      mdelta is delta for move_camera method. Control keys can be redefined in
      'controls' table (see source code). Also it calls world.cd:collide() after
      the camera was moved.
   ]]--

   -- Also 'framedelta' contains time in milliseconds taken to render previous frame
   voxutils.process_keyboard_movement (world, keystate, 0.25*framedelta)

   -- This is how mouse movement is handeled
   local mask, x, y = vs.getRelativeMouseState()
   local rot = {-y*0.0005*framedelta, 0, -x*0.0005*framedelta}
   world.camera:rotate_camera (rot)

   previous_time = time

   return true
end
