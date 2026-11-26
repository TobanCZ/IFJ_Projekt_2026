import ifj26.*

@main def main(): Unit =
    val i = fInt(5)
    val d = fDouble(i)
    val s = fString("go")
    val u = fUnit()
    val sd = toStr(d)
    print(i); print("|"); print(sd); print("|"); print(s); print("|"); print(u); print("\n")

def fInt(x: Int): Int = x * 3
def fDouble(x: Int): Double = x / 2.0
def fString(s: String): String = s + "!"
def fUnit(): Unit = ()
