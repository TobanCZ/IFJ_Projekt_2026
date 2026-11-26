import ifj26.*

def retU(u: Unit): Unit = u

@main def main(): Unit =
    val u1 = ()
    val u2 = retU(u1)
    val s = toStr(u2)
    print(s); print("|")
    if u1 == u2 then
        print("EQ")
    else
        print("NE")
    print("|")
    if u1 != () then
        print("NE")
    else
        print("EQ")
    print("\n")
