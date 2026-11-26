import ifj26.*

@main def main(): Unit =
    val a = 1e-2
    val b = 1.25e2
    val c = .125
    val d = 1_2.3_4e+1D
    val s1 = toStr(a)
    val s2 = toStr(b)
    val s3 = toStr(c)
    val s4 = toStr(d)
    print(s1); print("|"); print(s2); print("|"); print(s3); print("|"); print(s4); print("\n")
