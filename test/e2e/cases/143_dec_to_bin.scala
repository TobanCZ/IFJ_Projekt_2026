import ifj26.*

def toBin(n: Int): String =
    if n == 0 then
        return "0"
    else
        ()
    var x = n
    var res = ""
    while x > 0 do
        val half = x / 2
        val bit = x - half * 2
        val sb = toStr(bit)
        res = sb + res
        x = half
    res

@main def main(): Unit =
    val a = toBin(0)
    val b = toBin(5)
    val c = toBin(42)
    val d = toBin(255)
    print(a); print("|"); print(b); print("|"); print(c); print("|"); print(d); print("\n")
