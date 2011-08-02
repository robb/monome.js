NODE = node
name = all

total: build_native

build_native:
	$(MAKE) -C ./lib

clean_native:
	$(MAKE) -C ./lib clean

clean:
	rm ./lib/monome.node
	rm -r ./lib/build

.PHONY: total
