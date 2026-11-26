import ifj26.*

def helper(a: Int, b: String, c: Double): String =
    val sa = toStr(a)
    val sc = toStr(c)
    b + ":" + sa + ":" + sc

def many(p1: Int, p2: Int, p3: Double, p4: String, p5: Unit, p6: Int): String =
    val sum = p1 + p2 + p6
    val sub = helper(sum, p4, p3)
    print(p5)
    return sub

@main def main(): Unit =
    val u = ()
    val res = many(1, 2, 3.5, "ok", u, 4)
    print("|")
    print(res)
    print("\n")
