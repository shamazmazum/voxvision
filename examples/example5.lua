vt = voxtrees
vr = voxrnd
vs = voxsdl

function remove_sphere (tree, position)
   local i,j,k
   for i=-8,8 do
      for j=-8,8 do
         for k=-8,8 do
            local dist = i*i + j*j + k*k
            if dist <= 64 then
               local dot = {position[1]+i, position[2]+j, position[3]+k}
               tree:delete (dot)
            end
         end
      end
   end
end

function init (ctx)
   print "This example is a demonstration of value noise created with vn3d library"
   print "If you get stuck somewhere, you can remove voxels you looking at by clicking left mouse button."
   print "You can rebuild the tree by pressing 'r'."
   print "Generation of the noise takes some time."
   vn3d.randomize ()
   local gen = vn3d.value_noise_generator (4, 5, 5, 5)
   local val, dist

   local i,j,k
   local coord = {}
   local set = vt.dotset (100*100*1200)
   for i = 0,100 do
      coord[1] = i
      for j = 0,1200 do
         coord[2] = j
         for k = 0,100 do
            coord[3] = k
            local x = i-50
            local z = k-50
            dist = x*x + z*z
            if dist > 2000 and dist < 2500 then
               set:push (coord)
            elseif dist < 2000 then
               val = gen:getnoise (coord)
               if val > 0.55 then
                  set:push (coord)
               end
            end
         end
      end
   end
   local tree = vt.settree (set)
   print (string.format ("There is %i voxels in the tree", #tree))

   local camera = vr.camera "simple-camera"
   camera.position = {50, 600, 50}

   local cd = vr.cd()
   cd:attach_camera (camera, 2)
   cd:attach_context (ctx)

   ctx.tree = tree
   ctx.camera = camera
   ctx.cd = cd
   ctx.fps_controller = vr.fps_controller (30)
   ctx.fps_restricted = true
   return true
end

function tick (world, time)
   local event, quit
   local w, h = world:get_geometry()
   local tree = world.tree
   local camera = world.camera

   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown then
         if event.keysym.sym == vs.key.Escape then
            quit = true

         elseif event.keysym.sym == vs.key.f then
            if world.fps_restricted == true then
               world.fps_controller = vr.fps_controller (0)
               world.fps_restricted = nil
            else
               world.fps_controller = vr.fps_controller (30)
               world.fps_restricted = true
            end

         elseif event.keysym.sym == vs.key.r then
            print "Rebuilding tree"
            tree:rebuild()
         end

      elseif event.type == vs.event.Quit then
         quit = true

      elseif event.type == vs.event.MouseButtonDown then
         local dir = camera:screen2world (w/2, h/2)
         local position = camera:get_position ()
         local intersection = tree:ray_intersection (position, dir)
         if intersection then
            remove_sphere (tree, intersection)
         end
      end
   end
   if quit then return nil end

   local keystate = vs.getKeyboardState()

   previous_time = previous_time or time
   local framedelta = time - previous_time
   voxutils.process_keyboard_movement (world, keystate, 0.05*framedelta)
   local mask, x, y = vs.getRelativeMouseState()
   local rot = {-y*0.0005*framedelta, 0, -x*0.0005*framedelta}
   camera:rotate_camera (rot)
   previous_time = time

   local fps, upd = world.fps_controller:delay ()
   if upd then
      print (fps)
   end

   return true
end
