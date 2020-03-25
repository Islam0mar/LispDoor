# LispDoor

This is an **under construction** lisp1-dialect which is intended to be used on a microcontroller.

Forked from [FemtoLisp](https://github.com/JeffBezanson/femtolisp) and [ECL](https://gitlab.com/embeddable-common-lisp/ecl).


# Supported
- lambda, label, set
- fixnum, symbol, gensym(non-standard)

# TODO:
- [ ] complete primitive function
- [ ] support string,vector,bitvector
- [ ] add HAL functions
- [ ] add multitasking (maybe Round-robin scheduling)

# compilation

```shell
    mkdir build
    cd build
    cmake ..
    make
    make install
```
