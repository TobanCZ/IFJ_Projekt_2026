import ifj26.*

def f(x: Int): Int =
    if x > 0 then
        val x = 100
        print(x); print(" ")
    else
        print(x); print(" ")
    x

@main def main(): Unit =
    val a = f(5)
    val neg = -3
    val b = f(neg)
    print(a); print(" "); print(b); print("\n")
