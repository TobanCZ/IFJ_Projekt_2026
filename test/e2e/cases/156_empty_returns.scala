import ifj26.*

def retEmptyStr(): String =
    val s = ""
    s

def retUnitImplicit(u: Unit): Unit =
    val x = u
    ()

@main def main(): Unit =
    val s = retEmptyStr()
    val u = retUnitImplicit(())
    val len = strLength(s)
    print(len); print("|"); print(u); print("\n")
