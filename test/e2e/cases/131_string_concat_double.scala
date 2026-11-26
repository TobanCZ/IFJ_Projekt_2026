import ifj26.*

@main def main(): Unit =
    var s = "a"
    var i = 0
    while i < 5 do
        s = s + s
        i = i + 1
    val len = strLength(s)
    print(len); print(" "); print(s); print("\n")
