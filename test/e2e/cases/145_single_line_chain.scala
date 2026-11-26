import ifj26.*

def f1(x: Int) = f2(x)
def f2(x: Int): Int = x * 2
def f3(x: Int) = x + 10
def f4(x: Double): Double = x / 2.0
def f5(s: String) = s + "!"
def f6(): Int = 7

@main def main(): Unit =
    val v0 = f6()
    val v1 = f1(v0)
    val v2 = f3(v1)
    val v3 = f4(v2)
    val sv3 = toStr(v3)
    val v4 = f5(sv3)
    print(v4); print("\n")
