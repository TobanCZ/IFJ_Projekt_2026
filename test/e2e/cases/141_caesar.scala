import ifj26.*

def shiftChar(ch: String, k: Int): String =
    val o = ord(ch, 0)
    if o >= 65 then
        if o <= 90 then
            var shifted = o - 65 + k
            val q = shifted / 26
            shifted = shifted - q * 26 + 65
            return chr(shifted)
        else
            return ch
    else
        return ch

@main def main(): Unit =
    val text = "ABYZ_12"
    val len = strLength(text)
    var res = ""
    var i = 0
    while i < len do
        val ip1 = i + 1
        val ch = subStr(text, i, ip1)
        val enc = shiftChar(ch, 3)
        res = res + enc
        i = i + 1
    print(res); print("\n")
