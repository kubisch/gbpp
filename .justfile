default: clean build run

clean:
  rm -rf build

build:
  cmake -S . -B build
  cmake --build build  

run:
  ./build/gbpp