import ifj26.*

def isPal(s: String): Int =
    val len = strLength(s)
    var l = 0
    var r = len - 1
    while l < r do
        val cl = ord(s, l)
        val cr = ord(s, r)
        if cl != cr then
            return 0
        else
            l = l + 1
            r = r - 1
    1

@main def main(): Unit =
    val a = isPal("racecar")
    val b = isPal("hello")
    val c = isPal("a")
    val d = isPal("")
    val e = isPal("abba")
    print(a); print(b); print(c); print(d); print(e); print("\n")
