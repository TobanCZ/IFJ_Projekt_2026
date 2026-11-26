import ifj26.*

@main def main(): Unit =
    var s = ""
    var i = 0
    while i < 5 do
        val code = 65 + i
        val ch = chr(code)
        s = s + ch
        i = i + 1
    val part = subStr(s, 1, 4)
    val multi = """line1

#hash\slash"""
    print(s); print("|")
    print(part); print("|")
    print(multi); print("\n")
