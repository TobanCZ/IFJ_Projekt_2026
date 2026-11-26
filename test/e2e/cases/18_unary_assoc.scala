import ifj26.*

@main def main(): Unit =
    val a = 10 - 3 - 2
    val b = 10.0 / 2.0 / 2.0
    val c = -7 / 3
    val d = 7 / -3
    val e = -7 / -3
    val x = 4
    val y = 2
    val f = -(x + y) * -(x - y)
    val g = --(x + 1)
    val h = x - -y
    val sb = toStr(b)
    print(a); print("|")
    print(sb); print("|")
    print(c); print("|")
    print(d); print("|")
    print(e); print("|")
    print(f); print("|")
    print(g); print("|")
    print(h); print("\n")
