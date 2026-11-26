import ifj26.*

@main def main(): Unit =
    val c1 = chr(1)
    val c127 = chr(127)
    val o1 = ord(c1, 0)
    val o127 = ord(c127, 0)
    print(o1); print(" "); print(o127); print("\n")
