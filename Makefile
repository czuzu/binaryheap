all: build run

build:
	@echo
	@echo \*** Compiling and linking test app ...
	@echo
	mkdir -p out/
	gcc -I. binheap_test.c -o out/binheap_test.app
	chmod +x out/binheap_test.app

run:
	@echo
	@echo \*** Running test app ...
	@echo
	@./out/binheap_test.app
	@echo
	@echo
	@echo ALL TESTS PASSED!

clean:
	rm -rf out/

.PHONY: all build run clean
