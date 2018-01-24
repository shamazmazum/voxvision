local voxutils = {}
local scancodes = voxvision.voxsdl.scancode
local format = string.format
local print = print
_ENV = voxutils

default_controls = {
   forward = scancodes.W,
   backward = scancodes.S,
   left = scancodes.A,
   right = scancodes.D,
   up = scancodes.Z,
   down = scancodes.X,
}

function process_keyboard_movement (world, keystate, mdelta, controls)
   controls = controls or default_controls
   mdelta = mdelta or 5
   adelta = adelta or 0.05
   local camera = world.camera
   local cd = world.cd

   if keystate[controls.backward] then
      camera:move_camera {0,-mdelta,0}
   elseif keystate[controls.forward] then
      camera:move_camera {0,mdelta,0}
   end

   if keystate[controls.left] then
      camera:move_camera {-mdelta,0,0}
   elseif keystate[controls.right] then
      camera:move_camera {mdelta,0,0}
   end

   if keystate[controls.up] then
      camera:move_camera {0,0,mdelta}
   elseif keystate[controls.down] then
      camera:move_camera {0,0,-mdelta}
   end

   if cd then cd:collide() end
end

rendering_modes = {"best", "adaptive", "fast"}
function next_rendering_mode (context)
   current_rendering_mode = current_rendering_mode or 1
   local mode = rendering_modes[current_rendering_mode]
   print (format ("Setting %s rendering mode", mode))
   context:rendering_mode (mode)
   current_rendering_mode = current_rendering_mode + 1
   if (current_rendering_mode > #rendering_modes) then
      current_rendering_mode = 1
   end
end

ray_merge_modes = {"no", "accurate", "fast"}
function next_ray_merge_mode (context)
   current_rm_mode = current_rm_mode or 1
   local mode = ray_merge_modes [current_rm_mode]
   context:ray_merge_mode (mode)
   print (format ("Setting %s ray merge mode", mode))
   current_rm_mode = current_rm_mode + 1
   if (current_rm_mode > #ray_merge_modes) then
      current_rm_mode = 1
   end
end

return voxutils
