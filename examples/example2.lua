vt = voxtrees
vr = voxrnd
vs = voxsdl

function init (ctx)
   local test = function (sample) return sample > 40 end
   --[[
      Build tree from a raw file 'skull.dat' in voxvision's system-wide data
      directory. 'skull.dat' is provided as an example.
   ]]--
   local tree = vt.read_raw_data (vt.find_data_file "skull.dat", {256,256,256}, 1, test)
   print (#tree)

   local camera = vr.camera "simple-camera"
   camera.position = {100,60,-100}
   camera.rotation = {1.4, 0, 0}

   ctx.tree = tree
   ctx.camera = camera
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
   local camera = world.camera

   -- If 's' key is pressed, move camera, and so on
   if keystate[vs.scancode.S] then
      camera:move_camera {0,-5,0}
   elseif keystate[vs.scancode.W] then
      camera:move_camera {0,5,0}
   end

   if keystate[vs.scancode.A] then
      camera:move_camera {-5,0,0}
   elseif keystate[vs.scancode.D] then
      camera:move_camera {5,0,0}
   end

   if keystate[vs.scancode.Left] then
      camera:rotate_camera {0,0,0.05}
   elseif keystate[vs.scancode.Right] then
      camera:rotate_camera {0,0,-0.05}
   end
   
   if keystate[vs.scancode.Up] then
      camera:rotate_camera {-0.05,0,0}
   elseif keystate[vs.scancode.Down] then
      camera:rotate_camera {0.05,0,0}
   end

   return true
end
