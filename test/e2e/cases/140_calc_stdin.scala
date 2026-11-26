import ifj26.*

@main def main(): Unit =
    val n = readInt()
    var i = 0
    var acc = 0
    while i < n do
        val op = readInt()
        val val1 = readInt()
        if op == 1 then
            acc = acc + val1
        else
            if op == 2 then
                acc = acc - val1
            else
                acc = acc * val1
        i = i + 1
    print(acc); print("\n")
