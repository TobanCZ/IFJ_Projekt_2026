import ifj26.*

def harmonic(n: Int): Double =
    if n <= 1 then
        return 1.0
    else
        val nm1 = n - 1
        val prev = harmonic(nm1)
        return prev + 1.0 / n

@main def main(): Unit =
    val h = harmonic(5)
    val sh = toStr(h)
    print(sh); print("\n")
