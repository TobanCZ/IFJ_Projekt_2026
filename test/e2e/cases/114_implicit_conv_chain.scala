import ifj26.*

def addD(a: Double, b: Double): Double = a + b

@main def main(): Unit =
    var d: Double = 1
    d = d + 2
    val x = 3
    d = x
    val r = addD(x, 4)
    val sd = toStr(d)
    val sr = toStr(r)
    print(sd); print(" "); print(sr); print("\n")
