import ifj26.*

def scale(x: Double, factor: Double): Double = x * factor

@main def main(): Unit =
    val a = (1 + 2.5) * (4 - 1)
    val b = 2 * 3 + 0.5
    val c = 1 + 2 / 4.0
    val d1 = toInt(5.9)
    val neg59 = -5.9
    val d2 = toInt(neg59)
    val d = d1 + d2
    val n = 5
    val s1 = scale(3, 2)
    val s2 = scale(n, 1.5)
    val sa = toStr(a)
    val sb = toStr(b)
    val sc = toStr(c)
    val ss1 = toStr(s1)
    val ss2 = toStr(s2)
    print(sa); print("|")
    print(sb); print("|")
    print(sc); print("|")
    print(d); print("|")
    print(ss1); print("|")
    print(ss2); print("\n")
