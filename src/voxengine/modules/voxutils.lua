local voxutils = {}

local scancodes = voxvision.voxsdl.scancode
local rendering_modes = voxvision.voxrnd.rendering_modes

local print = print
local next = next

local yield = coroutine.yield
local wrap = coroutine.wrap
local format = string.format

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

function circlenext (t, key)
   local newkey, val = next (t, key)
   if newkey == nil then
      return next (t, nil)
   end

   return newkey, val
end

next_rendering_mode = wrap (function (context)
      local key, val
      for key, val in circlenext, rendering_modes do
         print (format ("Setting rendering mode to: %s", key))
         yield (context:rendering_mode (val))
      end
end)

return voxutils
