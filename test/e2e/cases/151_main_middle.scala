import ifj26.*

def before(x: Int): Int = x + 1

@main def startHere(): Unit =
    val a = before(10)
    val b = after(a)
    print(a); print(" "); print(b); print("\n")

def after(x: Int): Int = x * 2
