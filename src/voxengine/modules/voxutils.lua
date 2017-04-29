local voxutils = {}
local scancodes = voxvision.voxsdl.scancodes
local dot = voxvision.voxtrees.dot
_ENV = voxutils

default_controls = {
   forward = scancodes.w,
   backward = scancodes.s,
   left = scancodes.a,
   right = scancodes.d,
   up = scancodes.z,
   down = scancodes.x,
   lookup = scancodes.uparrow,
   lookdown = scancodes.downarrow,
   turnleft = scancodes.leftarrow,
   turnright = scancodes.rightarrow
}

function process_movement (keystate, camera, mdelta, adelta, controls)
   controls = controls or default_controls
   mdelta = mdelta or 5
   adelta = adelta or 0.05

   if keystate:keypressed (controls.backward) then
      camera:move_camera (dot (0,-mdelta,0))
   elseif keystate:keypressed (controls.forward) then
      camera:move_camera (dot (0,mdelta,0))
   end

   if keystate:keypressed (controls.left) then
      camera:move_camera (dot (-mdelta,0,0))
   elseif keystate:keypressed (controls.right) then
      camera:move_camera (dot (mdelta,0,0))
   end

   if keystate:keypressed (controls.up) then
      camera:move_camera (dot (0,0,mdelta))
   elseif keystate:keypressed (controls.down) then
      camera:move_camera (dot (0,0,-mdelta))
   end

   if keystate:keypressed (controls.turnleft) then
      camera:rotate_camera (dot (0,0,adelta))
   elseif keystate:keypressed (controls.turnright) then
      camera:rotate_camera (dot (0,0,-adelta))
   end
   
   if keystate:keypressed (controls.lookup) then
      camera:rotate_camera (dot (-adelta,0,0))
   elseif keystate:keypressed (controls.lookdown) then
      camera:rotate_camera (dot (adelta,0,0))
   end
end

return voxutils
