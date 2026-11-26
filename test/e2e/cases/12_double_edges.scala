import ifj26.*

@main def main(): Unit =
    val a = .5
    val b = .0
    val c = 1e0
    val d = 1e+2
    val e = 2.5e-1D
    val f: Double = 3
    val g = f / 2
    val h = 7 / 2
    val s1 = toStr(a)
    val s2 = toStr(b)
    val s3 = toStr(c)
    val s4 = toStr(d)
    val s5 = toStr(e)
    val s6 = toStr(g)
    print(s1); print("|")
    print(s2); print("|")
    print(s3); print("|")
    print(s4); print("|")
    print(s5); print("|")
    print(s6); print("|")
    print(h); print("\n")
