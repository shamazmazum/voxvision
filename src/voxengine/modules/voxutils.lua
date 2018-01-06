local voxutils = {}
local scancodes = voxvision.voxsdl.scancode
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

return voxutils
