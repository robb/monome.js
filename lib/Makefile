name = all
JOBS = 1

all:
	rm -rf build .lock-wscript monome.node
	node-waf configure build

clean:
	rm -rf build .lock-wscript monome.node

.PHONE: all
