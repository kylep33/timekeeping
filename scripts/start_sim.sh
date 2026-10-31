CFLAGS=-g emmake make BOARD=sensorwatch_pro DISPLAY=custom
python3 -m http.server -d build-sim
# -> http://localhost:8000/firmware.html
