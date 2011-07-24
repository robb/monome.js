{Monome} = require 'monome'

class Grid
  constructor: (@monome) ->
    [@width, @height] = [@monome.rows, @monome.columns]

    @grid    = new Array @height
    @grid[i] = new Array @width for i in [0..@height]

    for x in [0..@width]
      for y in [0..@height]
        @grid[x][y] = off

  step: ->
    newGrid    = new Array @height
    newGrid[i] = new Array @width for i in [0..@height]

    getNeighbours = (x, y) =>
      neighbours = 0
      for offsetX in [-1..1]
        for offsetY in [-1..1]
          continue if offsetX is offsetY is 0

          neighbours++ if @grid[(x + offsetX + @width) % @width][(y + offsetY + @height) % @height]

      neighbours

    for x in [0..@width]
      for y in [0..@height]
        neighbours = getNeighbours x, y
        if @grid[x][y]
          newGrid[x][y] = 2 <= neighbours <= 3
        else
          newGrid[x][y] = neighbours is 3

    @grid = newGrid

  draw: ->
    for x in [0..@width]
      for y in [0..@height]
        @monome.set x, y, @grid[x][y]

  toggle: (x, y) ->
    if @grid[x]?[y]?
      @grid[x][y] = !@grid[x][y]

monome = new Monome '/dev/tty.usbserial-m64-0123'
grid   = new Grid monome

tick = ->
  grid.step()
  grid.draw()

  setTimeout tick, 50

monome.onbuttondown = (x, y) ->
  grid.toggle x, y
  grid.draw()

monome.onbuttonup = ->

monome.start()
tick()