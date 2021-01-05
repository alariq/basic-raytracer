rm -rf ./build
mkdir ./build
cmake -DCMAKE_BUILD_TYPE=Release  -B build/ -S . 
cd ./build && make


