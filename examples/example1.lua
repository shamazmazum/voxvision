vt = voxtrees
vr = voxrnd
vs = voxsdl

function init (ctx)
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

   --[[
      This is another way to create the same tree. Do not worry about freeing
      the previous tree, GC will do it for us.
   ]]--
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

   --[[
      These methods are like methods in vox_camera_interface.
      You can set position and FOV using dot notation, needed property setters
      will be called automatically.
   ]]--
   camera.position = {25,-100,25}
   camera.fov = 0.45
   camera:look_at {25,25,25}

   -- You must add at leat two elements to the context: a tree and a camera.
   ctx.tree = t
   ctx.camera = camera
   return true
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
   -- Return false when you wish to shut down the engine or true otherwise
   if quit then return false end

   -- You can rotate the camera or modify the tree in tick() function
   local camera = world.camera
   time = time / 2000
   local dot = {25+125*math.sin(time),25-125*math.cos(time),25}
   camera.position = dot
   camera:look_at {25,25,25}

   return true
end
