import ifj26.*

def loud(): Int =
    print("L")
    42

def u1(): Unit =
    return loud()

def u2(): Unit = print("S")

def u3(): Unit =
    val x = loud()
    x + 1

@main def main(): Unit =
    val a = u1()
    val b = u2()
    val c = u3()
    print(a); print(b); print(c); print("\n")
