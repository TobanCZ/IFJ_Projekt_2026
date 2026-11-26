import ifj26.*

def fn1(x: Int): Int = x + 1
def fn2(x: Int): Int = x + 2
def fn3(x: Int): Int = x + 3
def fn4(x: Int): Int = x + 4
def fn5(x: Int): Int = x + 5
def fn6(x: Int): Int = x + 6
def fn7(x: Int): Int = x + 7
def fn8(x: Int): Int = x + 8
def fn9(x: Int): Int = x + 9
def fn10(x: Int): Int = x + 10
def fn11(x: Int): Int = x + 11
def fn12(x: Int): Int = x + 12

def chain(x: Int): Int =
    var r = fn1(x)
    r = fn2(r); r = fn3(r); r = fn4(r); r = fn5(r); r = fn6(r)
    r = fn7(r); r = fn8(r); r = fn9(r); r = fn10(r); r = fn11(r); r = fn12(r)
    r

@main def main(): Unit =
    val r = chain(0)
    print(r); print("\n")
