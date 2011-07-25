{Monome} = require 'monome'

monome = new Monome '/dev/tty.usbserial-m64-0123'

console.log monome

monome.onbuttondown = (x, y, i) ->
  monome.set x, y, on

monome.onbuttonup = (x, y, i) ->
  monome.set x, y, off

monome.start()
