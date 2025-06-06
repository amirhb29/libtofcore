help:
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'

PHONY: build
build:   ## build libtofcore
	cmake -B build
	cmake --build build

PHONY: install
install:   ## isntall libtofcore
	cd build && sudo make install

PHONY: pytofcore
pytofcore: ## install pytofcore (need to run make build first)
	python3 -m pip uninstall -y pytofcore
	cp build/tofcore/wrappers/python/*.so tofcore/wrappers/python/pytofcore
	cd tofcore/wrappers/python && python3 setup.py install --user
	ln -sf ./build/tofcore/wrappers/python/pytofcore.cpython*.so  


PHONY: functional_tests
functional_tests: ## runs python functional tests (needs pytofcore and pytofcrust installed and oasis device connected to PC)
	python3 -m pytest -m "functional" -v .

PHONY: unit_tests
unit_tests: ## runs unit tests (make sure no oasis is connected to PC)
	python3 -m pytest -m "not functional and not sdram_selftest" -v .


PHONY : cpp_unit_test
cpp_unit_test:
	ctest --test-dir ./build/tofcore 


PHONY: clean
clean: ## cleans environment of build artifacts
	python3 -m pip uninstall -y pytofcore
	rm -rf build
	rm -f tofcore/wrappers/python/pytofcore/*.so
	rm -rf tofcore/wrappers/python/*.egg-info
	rm -rf tofcore/wrappers/python/build
	rm -rf tofcore/wrappers/python/dist

