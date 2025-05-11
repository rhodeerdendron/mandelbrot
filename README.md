# mandelbrot

## Building

```sh
# clone with submodules
git clone https://github.com/rhodeerdendron/mandelbrot.git --recurse-submodules --depth=1

mkdir build
cd build

# use your preferred build system
cmake .. -G ninja
ninja

# run executable from base directory
cd ..
build/mandelbrot
```

