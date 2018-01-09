-- This example explains how to modify the tree "online", i.e. when the render
-- is working.

-- Shortcuts as always
vt = voxtrees
vr = voxrnd
vs = voxsdl

-- Create an empty table which will help us to track created voxels
n = {}

function init ()
   -- An empty tree
   local tree = vt.tree()
   -- "Doom" camera is just like in the good old Doom. It's a little faster then
   -- simple-camera   
   local camera = vr.camera "doom-camera"
   camera:set_position {50, -50, -20}

   -- Initialize n with empty tables
   local i
   for i = 1,100 do
      n[i] = {}
   end

   local light_manager = vr.light_manager ()
   light_manager:insert_shadowless_light ({50, 100, 100}, 150, {1, 1, 1})

   -- Here we return not a tree, but a so-called scene proxy. It replaces
   -- dangerous calls like rebuild or insert with thread-safe equivalents.
   return {tree = vr.scene_proxy (tree), camera = camera, light_manager = light_manager}
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

   -- Get a random dot on XY plane
   local x,y = math.random (100), math.random (100)
   local height = n[x][y] or 0
   -- Insert a dot to the tree
   world.tree:insert {x, y, height}
   -- Update height table
   n[x][y] = height - 1

   rebuild_time = rebuild_time or time
   if time - rebuild_time > 10000 then
      print "Rebuilding the tree"
      -- Rebuild the tree asynchronously and in thread-safe manner
      world.tree:rebuild ()
      rebuild_time = time
   end

   return true
end
