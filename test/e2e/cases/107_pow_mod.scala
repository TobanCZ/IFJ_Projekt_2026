import ifj26.*

def mod(a: Int, m: Int): Int =
    val q = a / m
    a - q * m

def powMod(base: Int, exp: Int, m: Int): Int =
    var res = 1
    var b = mod(base, m)
    var e = exp
    while e > 0 do
        val half = e / 2
        if half * 2 != e then
            val prod = res * b
            res = mod(prod, m)
        else
            ()
        val sq = b * b
        b = mod(sq, m)
        e = half
    res

@main def main(): Unit =
    val r1 = powMod(2, 10, 1000)
    val r2 = powMod(3, 13, 100)
    print(r1); print(" "); print(r2); print("\n")
