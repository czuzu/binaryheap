all:
	@echo
	@echo \*** Compiling and linking test app ...
	@echo
	mkdir -p out/
	gcc -g -O0 -I. binheap_test.c -o out/binheap_test.app
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

.PHONY: all run clean
