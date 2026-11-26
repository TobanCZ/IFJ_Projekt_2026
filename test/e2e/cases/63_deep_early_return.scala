import ifj26.*

def search(target: Int): Int =
    var i = 0
    while i < 5 do
        var j = 0
        while j < 5 do
            if i * 10 + j == target then
                return i + j
            else
                j = j + 1
        i = i + 1
    return -1

def branchOrImplicit(x: Int): Int =
    if x < 0 then
        return 0
    else
        ()
    x * 2

@main def main(): Unit =
    val a = search(23)
    val b = search(99)
    val neg5 = -5
    val c = branchOrImplicit(neg5)
    val d = branchOrImplicit(7)
    print(a); print("|")
    print(b); print("|")
    print(c); print("|")
    print(d); print("\n")
