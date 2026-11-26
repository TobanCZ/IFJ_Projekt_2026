import ifj26.*

def incPrint(x: Int, tag: String): Int =
    print(tag)
    x + 1

@main def main(): Unit =
    val a = incPrint(1, "A")
    val b = incPrint(a, "B")
    val c = incPrint(b, "C")
    print("|"); print(c); print("\n")
