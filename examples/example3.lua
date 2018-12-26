vt = voxtrees
vr = voxrnd
vs = voxsdl

function init (ctx)
   --[[
      Build tree from a raw file 'skull.dat' in voxvision's system-wide data directory.
      read_raw_data_ranged() is like faster read_raw_data() with test function checking
      min <= sample < max (in current example sample >= 40)
   ]]--
   print (ctx:get_geometry())
   local tree = vt.read_raw_data_ranged (vt.find_data_file "skull.dat", {256,256,256}, 1, 40)
   print (#tree)

   local camera = vr.camera "simple-camera"
   camera.position = {100,60,-100}
   camera.rotation = {1.4, 0, 0}

   -- Also create a collision detector
   local cd = vr.cd()
   -- Attach the context and your camera. 4 is the camera's body radius
   cd:attach_camera (camera, 4)
   cd:attach_context (ctx)

   -- Do not forget to add collision detector to the context to prevent GC'ing.
   ctx.tree = tree
   ctx.camera = camera
   ctx.cd = cd
   -- This example creates FPS controller which limits frames per second amount by 60.
   ctx.fps_controller = vr.fps_controller (60)
   ctx.fps_restricted = true
end

function tick (world, time)
   local event, quit
   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown and event.keysym.sym == vs.key.Escape then
         quit = true

      elseif event.type == vs.event.Quit then
         quit = true

      elseif event.type == vs.event.KeyDown and event.keysym.sym == vs.key.f then
         -- Toggle FPS control
         if world.fps_restricted == true then
            world.fps_controller = vr.fps_controller (0)
            world.fps_restricted = nil
         else
            world.fps_controller = vr.fps_controller (60)
            world.fps_restricted = true
         end
      end
   end
   if quit then return nil end

   -- Get keyboard state
   local keystate = vs.getKeyboardState()

   previous_time = previous_time or time
   local framedelta = time - previous_time
   --[[
      This is keyboard events processing code. 'voxutils' module has a function
      `process_keyboard_movement` to process basic keyboard movement in one line
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

   --[[
      Delay method of FPS controller must be called once somewhere in tick() function.
      It returns actual FPS value which is updated once per second. When it is updated,
      update flag (true) is returned as the second value.
   ]]--

   local fps, upd = world.fps_controller:delay ()
   if upd then
      print (fps)
   end

   return true
end
