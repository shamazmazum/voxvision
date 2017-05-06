vt = voxtrees
vr = voxrnd

function init ()
   -- Set voxel size
   vt.voxelsize (vt.dot (0.25, 0.25, 0.25))
   -- Create a set for up to 50*50*50 dots
   local a = vt.dotset(50*50*50)
   for i = 0,50 do
      for j = 0,50 do
         for k = 0,50 do
            -- Push a new dot to the set
            a:push (vt.dot (i,j,k))
         end
      end
   end
   -- Convert the set to a voxel tree
   local t = vt.settree (a)
   -- How many dots are in the tree?
   print (#t)

   -- Create a new simple camera
   local camera = vr.simple_camera()
   -- These methods are like methods in vox_camera_interface
   camera:set_position (vt.dot (25,-100,25))
   camera:look_at (vt.dot (25,25,25))
   camera:set_fov (0.45)
   -- You must return 2 values from init: a tree and a camera
   return t, camera
end

function tick (tree, camera, time)
   -- You can rotate camera or modify tree in tick() function
   time = time / 2000
   local dot = vt.dot (25+125*math.sin(time),25-125*math.cos(time),25)
   camera:set_position (dot)
   camera:look_at (vt.dot (25,25,25))
end
