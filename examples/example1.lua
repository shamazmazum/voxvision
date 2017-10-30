vt = voxtrees
vr = voxrnd
vs = voxsdl

function init ()
   -- Set voxel size
   vt.voxelsize {0.125, 0.125, 0.125}
   -- Create a set for up to 50*50*50 dots
   local a = vt.dotset(50*50*50)
   for i = 0,50 do
      for j = 0,50 do
         for k = 0,50 do
            -- Push a new dot to the set
            a:push {i,j,k}
         end
      end
   end
   -- Convert the set to a voxel tree
   local t = vt.settree (a)
   -- How many dots are in the tree?
   print (#t)

   -- This is another way to create the same tree. Do not worry about freeing
   -- the previous tree, GC will do it for us.
   t = vt.tree() -- Empty tree
   for i = 0,50 do
      for j = 0,50 do
         for k = 0,50 do
            -- Push a new dot to the set
            t:insert {i,j,k}
         end
      end
   end
   -- Rebalance the tree
   t:rebuild()

   -- Create a new simple camera
   local camera = vr.camera "simple-camera"
   -- These methods are like methods in vox_camera_interface
   camera:set_position {25,-100,25}
   camera:look_at {25,25,25}
   camera:set_fov (0.45)

   --[[
      You must return 2 values from init: a tree and a camera. They must be put
      in a table, which is called the world table.
   --]]
   return {tree = t, camera = camera}
end

function tick (world, time)
   -- This is how events are handeled. Just like in C program
   local event, quit
   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown and event.keysym.sym == vs.key.Escape then
         quit = true
      elseif event.type == vs.event.Quit then
         quit = true
      end
   end
   -- Call request_quit when you wish to shut down the engine
   if quit then request_quit() end

   -- You can rotate camera or modify tree in tick() function
   local camera = world.camera
   time = time / 2000
   local dot = {25+125*math.sin(time),25-125*math.cos(time),25}
   camera:set_position (dot)
   camera:look_at {25,25,25}
end
