import ifj26.*

def alpha(x: Int): Int =
    var res = x * 10
    if x > 0 then
        val m = x - 1
        val tmp = beta(m)
        res = res + tmp
    else
        ()
    res

def beta(x: Int): Int =
    var res = x + 1
    if x > 0 then
        val m = x - 1
        val tmp = alpha(m)
        res = res + tmp
    else
        ()
    res

@main def main(): Unit =
    val x = alpha(4)
    print(x); print("\n")
