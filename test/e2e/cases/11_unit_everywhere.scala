import ifj26.*

def takeUnit(u: Unit): Unit =
    print(u)

def sideEffect(): Unit =
    print("side")
    return 99

@main def main(): Unit =
    val u: Unit = ()
    takeUnit(u)
    print(" ")
    val s = sideEffect()
    print(" ")
    print(s)
    print(" ")
    if () == () then
        if () != () then
            print("NO")
        else
            print("EQ")
    else
        print("NO")
    print("\n")
