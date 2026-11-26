import ifj26.*

@main def main(): Unit =
    var i = 32
    var ok = 1
    while i <= 126 do
        val c = chr(i)
        val o = ord(c, 0)
        if o != i then
            ok = 0
        else
            ()
        i = i + 1
    print(ok); print("\n")
