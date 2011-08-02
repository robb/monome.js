#!/bin/bash
if [ x`which gmake` != "x" ]; then
  echo "Using GNU make";

  gmake total
else
  make total
fi
