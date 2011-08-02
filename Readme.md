# Monone.js

Monome.js is a thin wrapper around libmonome.

## Example
Here's a quick example how to get started (in CoffeeScript).

    {Monome} = require 'monome'
    monome = new Monome '/dev/tty.usbserial-m64-0123'

    monome.onButtonDown = ({x, y}) ->
      monome.setLED x, y, on

    monome.onButtonUp = ({x, y}) ->
      monome.setLED x, y, off

    monome.start()'
