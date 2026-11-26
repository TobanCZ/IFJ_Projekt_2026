import ifj26.*

def findFirstEven(n: Int): Int =
    var i = n
    while i < 20 do
        val half = i / 2
        if half * 2 == i then
            return i
        else
            i = i + 1
    return -1

def nestedIfReturn(x: Int): Int =
    if x < 0 then
        return -1
    else
        if x == 0 then
            return 0
        else
            return 1
    val unreachable = 999
    unreachable

@main def main(): Unit =
    val a = findFirstEven(5)
    val b = findFirstEven(25)
    val neg10 = -10
    val c = nestedIfReturn(neg10)
    val d = nestedIfReturn(0)
    val e = nestedIfReturn(10)
    print(a); print(" ")
    print(b); print(" ")
    print(c); print(" ")
    print(d); print(" ")
    print(e); print("\n")
