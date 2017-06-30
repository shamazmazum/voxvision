local voxutils = {}
local scancodes = voxvision.voxsdl.scancode
local dot = voxvision.voxtrees.dot
_ENV = voxutils

default_controls = {
   forward = scancodes.W,
   backward = scancodes.S,
   left = scancodes.A,
   right = scancodes.D,
   up = scancodes.Z,
   down = scancodes.X,
}

function process_keyboard_movement (keystate, camera, mdelta, controls)
   controls = controls or default_controls
   mdelta = mdelta or 5
   adelta = adelta or 0.05

   if keystate[controls.backward] then
      camera:move_camera (dot (0,-mdelta,0))
   elseif keystate[controls.forward] then
      camera:move_camera (dot (0,mdelta,0))
   end

   if keystate[controls.left] then
      camera:move_camera (dot (-mdelta,0,0))
   elseif keystate[controls.right] then
      camera:move_camera (dot (mdelta,0,0))
   end

   if keystate[controls.up] then
      camera:move_camera (dot (0,0,mdelta))
   elseif keystate[controls.down] then
      camera:move_camera (dot (0,0,-mdelta))
   end
end

return voxutils
