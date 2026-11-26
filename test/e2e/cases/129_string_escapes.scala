import ifj26.*

@main def main(): Unit =
    val s = "\\\"\n\r\t# "
    val len = strLength(s)
    print(len); print("|")
    val o0 = ord(s, 0)
    val o1 = ord(s, 1)
    val o2 = ord(s, 2)
    val o3 = ord(s, 3)
    val o4 = ord(s, 4)
    val o5 = ord(s, 5)
    val o6 = ord(s, 6)
    print(o0); print(","); print(o1); print(","); print(o2); print(",")
    print(o3); print(","); print(o4); print(","); print(o5); print(","); print(o6)
    print("\n")
