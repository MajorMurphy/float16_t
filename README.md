# float16_t

`float16_t` is a header only c++ library for [half-precision floating-point format](https://en.wikipedia.org/wiki/Half-precision_floating-point_format).


----

## Usage:

**include the header file**

```cpp
#include "float16_t.hpp"
using numeric::float16_t;
```

then compile using a C++20 compatible compiler with command line like:
```
clang++ -c -std=c++2a -Wall -Wextra -ferror-limit=1 -ftemplate-backtrace-limit=0 -funsafe-math-optimizations  -Ofast -flto -pipe -march=native -DDEBUG -o ./obj/test_test.o tests/test.cc
```

if you do not want to dump binary information, you can ommit the `-DDEBUG` option.


Example code:

```cpp
    float16_t f = 1.1f;
    {
        std::cout << "f =" << f << std::endl;
        std::cout << "float(f) =" << float(f) << std::endl;
        std::cout << "f++ =" << f++ << std::endl;
        std::cout << "++f =" << ++f << std::endl;
        std::cout << "f-- =" << f-- << std::endl;
        std::cout << "--f =" << --f << std::endl;
    }

```
this produces

```
f =1.09961(0 01111 0001100110)
float(f) =1.09961
f++ =1.09961(0 01111 0001100110)
++f =3.09961(0 10000 1000110011)
f-- =3.09961(0 10000 1000110011)
--f =1.09961(0 01111 0001100110)
```

And

```cpp
    float16_t g = -1.3f;
    {
        std::cout << "f+g = " << f+g << std::endl;
        std::cout << "f-g = " << f-g << std::endl;
        std::cout << "f*g = " << f*g << std::endl;
        std::cout << "f/g = " << f/g << std::endl;
    }

```
produces

```
f+g = -0.200195(1 01100 1001101000)
f-g = 2.39844(0 10000 0011001100)
f*g = -1.42969(1 01111 0110111000)
f/g = -0.846191(1 01110 1011000101)
```

Moreover,

```cpp
    float16_t h = 0.27f;
    {
        std::cout << "numeric::sin(h) = " << numeric::sin(h) << std::endl;
        std::cout << "numeric::cos(h) = " << numeric::cos(h) << std::endl;
        std::cout << "numeric::sqrt(h) = " << numeric::sqrt(h) << std::endl;
        std::cout << "numeric::cbrt(h) = " << numeric::cbrt(h) << std::endl;
    }

```

produces

```
numeric::sin(h) = 0.266846(0 01101 0001000101)
numeric::cos(h) = 0.963867(0 01110 1110110110)
numeric::sqrt(h) = 0.519531(0 01110 0000101000)
numeric::cbrt(h) = 0.646484(0 01110 0100101100)

```

There are also some predefined constants:

```cpp
    using namespace numeric;
    std::cout << "fp16_infinity:\t" << fp16_infinity << std::endl;
    std::cout << "fp16_max:\t" << fp16_max << std::endl;
    std::cout << "fp16_max_subnormal:\t" << fp16_max_subnormal << std::endl;
    std::cout << "fp16_min:\t" << fp16_min << std::endl;
    std::cout << "fp16_min_positive:\t" << fp16_min_positive << std::endl;
    std::cout << "fp16_min_positive_subnormal:\t" << fp16_min_positive_subnormal << std::endl;
    std::cout << "fp16_nan:\t" << fp16_nan << std::endl;
    std::cout << "fp16_infinity_negative:\t" << fp16_infinity_negative << std::endl;

    std::cout << "fp16_one:\t" << fp16_one << std::endl;
    std::cout << "fp16_zero:\t" << fp16_zero << std::endl;
    std::cout << "fp16_zero_negative:\t" << fp16_zero_negative << std::endl;
    std::cout << "fp16_e:\t" << fp16_e << std::endl;
    std::cout << "fp16_pi:\t" << fp16_pi << std::endl;
    std::cout << "NAN:\t" <<  float16_t{std::numeric_limits<float>::quiet_NaN()} << std::endl;
```

produces

```
fp16_infinity:  inf(0 11111 0000000000)
fp16_max:       65504(0 11110 1111111111)
fp16_max_subnormal:     0.000122011(0 00000 1111111111)
fp16_min:       -65504(1 11110 1111111111)
fp16_min_positive:      6.10352e-05(0 00001 0000000000)
fp16_min_positive_subnormal:    6.10948e-05(0 00000 0000000001)
fp16_nan:       nan(0 11111 1000000000)
fp16_infinity_negative: -inf(1 11111 0000000000)
fp16_one:       1(0 01111 0000000000)
fp16_zero:      0(0 00000 0000000000)
fp16_zero_negative:     -0(1 00000 0000000000)
fp16_e: 2.71875(0 10000 0101110000)
fp16_pi:        3.14062(0 10000 1001001000)
E:      2.71875(0 10000 0101110000)
PI:     3.14062(0 10000 1001001000)
NAN:    nan(0 11111 1000000000)
```



For more information, please check out the source file `float16_t.hpp`.


## Acknowledgements:

+ [half float](https://github.com/acgessler/half_float)
+ [float16](https://github.com/x448/float16)

## License

BSD License, Anti-996 License.

