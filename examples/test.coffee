{Monome} = require 'monome'

unless path = process.argv[2]
  console.log "Usage: coffee test.coffee path_to_monome"
  return 1

monome = new Monome path

console.log monome
console.log monome.__proto__

monome.onButtonDown = ({x, y}) ->
  monome.setLED x, y, on

monome.onButtonUp = ({x, y}) ->
  monome.setLED x, y, off

encoderState = [off, off, off, off]
monome.onEncoderDown = ({number}) ->
  encoderState[number] = yes

  monome.setRing number, 0x0F
  monome.setRing number, encoderPosition[number] / 4, 0x00

monome.onEncoderUp = ({number}) ->
  encoderState[number] = no

  monome.setRing number, 0x00
  monome.setRing number, encoderPosition[number] / 4, 0x0F

encoderPosition = [0, 0, 0, 0]
monome.onEncoderRotate = ({number, delta}) ->
  encoderPosition[number] = (encoderPosition[number] + delta) % 256

  if encoderState[number]
    monome.setRing number, 0x0F
    monome.setRing number, encoderPosition[number] / 4, 0x00
  else
    monome.setRing number, 0x00
    monome.setRing number, encoderPosition[number] / 4, 0x0F

monome.start()

