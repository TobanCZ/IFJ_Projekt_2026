import ifj26.*

@main def main(): Unit =
    val a = -(1 + -(2 + -(3 + -4)))
    val b = ---5
    val c = -(-(-1.5))
    val x = 10
    val y = 3
    val z = 2
    val d = x - -y - -z
    val sc = toStr(c)
    print(a); print("|"); print(b); print("|"); print(sc); print("|"); print(d); print("\n")
