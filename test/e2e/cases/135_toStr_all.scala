import ifj26.*

@main def main(): Unit =
    val n42 = -42
    val f0 = -1.234
    val s1 = toStr(0)
    val s2 = toStr(n42)
    val s3 = toStr(0.0)
    val s4 = toStr(f0)
    val s5 = toStr(99.999)
    val s6 = toStr("hi")
    val s7 = toStr(())
    print(s1); print("|"); print(s2); print("|"); print(s3); print("|")
    print(s4); print("|"); print(s5); print("|"); print(s6); print("|"); print(s7); print("\n")
