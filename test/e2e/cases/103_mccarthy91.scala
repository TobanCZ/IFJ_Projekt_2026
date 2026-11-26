import ifj26.*

def mc91(n: Int): Int =
    if n > 100 then
        return n - 10
    else
        val n1 = n + 11
        val inner = mc91(n1)
        val r = mc91(inner)
        return r

@main def main(): Unit =
    val a = mc91(87)
    val b = mc91(99)
    val c = mc91(100)
    val d = mc91(101)
    val e = mc91(115)
    print(a); print(" "); print(b); print(" "); print(c); print(" "); print(d); print(" "); print(e); print("\n")
