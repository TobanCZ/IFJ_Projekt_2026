import ifj26.*

@main def main(): Unit =
    var pi: Double = 3.0
    var sign: Double = 1.0
    var i = 2
    while i <= 100 do
        val denom = i * (i + 1) * (i + 2)
        pi = pi + sign * (4.0 / denom)
        sign = -sign
        i = i + 2
    val s = toStr(pi)
    print(s); print("\n")
