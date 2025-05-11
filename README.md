# mandelbrot

## Demo

The [Mandelbrot Set](https://en.wikipedia.org/wiki/Mandelbrot_set) is the set
of all complex numbers which the recursive function $$f(c)$$ does not diverge
to infinity. The function is defined as
$$f(c) = z => z^2 + c \text{ where } z_{initial} = 0+0i$$.

For purposes of computation, since all points with a magnitude greater than 2
will diverge, "diverges to infinity" is defined as when $$|z| > 2$$. Points on
the complex plane are colored based on how many iterations of $$f(c)$$ can occur
before the magnitude of $$z$$ crosses the chosen threshold.

The Mandelbrot Set is a special case of the family of
[Julia Sets](https://en.wikipedia.org/wiki/Julia_set), which exhibit similar
fractal behavior at their boundary.

Controls:
- WASD move center
- QE change zoom
- [] change exponent
- -+ change threshold

## Building

Requires OpenGL, GLEW, SDL2, and SDL2_ttf. If any library (other than OpenGL,
which is driver-specific) cannot be found, it will be built from source from
the submodules included in this repo.

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
