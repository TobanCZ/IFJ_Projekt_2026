import ifj26.*

def reverseStr(s: String): String =
    val len = strLength(s)
    var res = ""
    var i = len - 1
    while i >= 0 do
        val code = ord(s, i)
        val ch = chr(code)
        res = res + ch
        i = i - 1
    res

@main def main(): Unit =
    val s = "IFJ26_TEST"
    val rev = reverseStr(s)
    print(rev); print("\n")
