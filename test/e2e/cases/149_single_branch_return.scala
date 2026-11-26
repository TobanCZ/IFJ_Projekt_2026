import ifj26.*

def test(x: Int): Int =
    var y = x
    if y < 0 then
        return 0
    else
        y = y + 10
    y * 2

@main def main(): Unit =
    val n = -3
    val a = test(n)
    val b = test(5)
    print(a); print(" "); print(b); print("\n")
