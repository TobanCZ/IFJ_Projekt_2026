import ifj26.*

@main def main(): Unit =
    val a = 2 + 3 * 4
    val b = (2 + 3) * 4
    val c = -5 / 2
    val d = -5.0 / 2
    val e: Double = 7
    val f = --a
    val g = 1_000 + .5 + 1.5e2 + 2e-1D
    val sd = toStr(d)
    val se = toStr(e)
    val sg = toStr(g)
    print(a); print(" ")
    print(b); print(" ")
    print(c); print(" ")
    print(sd); print(" ")
    print(se); print(" ")
    print(f); print(" ")
    print(sg); print("\n")
