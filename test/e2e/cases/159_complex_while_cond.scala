import ifj26.*

@main def main(): Unit =
    var a = 1
    var b = 20
    var count = 0
    while a * 2 + 1 < b - 3 do
        a = a + 2
        b = b - 1
        count = count + 1
    print(count); print(" "); print(a); print(" "); print(b); print("\n")
