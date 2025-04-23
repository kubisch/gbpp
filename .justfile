default: clean build run

clean:
  rm -rf build

build:
  cmake -S . -B build
  cmake --build build  

run:
  ./build/gbpp


test: clean build
  cd build && ctest --output-on-failure

test-basic: build
  ./build/TestBasicOps