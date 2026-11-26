import ifj26.*

def fib(n: Int): Int =
    if n <= 1 then
        return n
    else
        val n1 = n - 1
        val n2 = n - 2
        val a = fib(n1)
        val b = fib(n2)
        return a + b

def sum3(a: Int, b: Int, c: Int): Int =
    val s = a + b + c
    s

@main def main(): Unit =
    val f0 = fib(0)
    val f1 = fib(1)
    val f7 = fib(7)
    val s = sum3(f0, f1, f7)
    print(f0); print(" ")
    print(f1); print(" ")
    print(f7); print(" ")
    print(s); print("\n")
