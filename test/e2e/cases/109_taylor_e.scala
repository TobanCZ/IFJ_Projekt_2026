import ifj26.*

@main def main(): Unit =
    var sum: Double = 1.0
    var term: Double = 1.0
    var i = 1
    while i <= 12 do
        term = term / i
        sum = sum + term
        i = i + 1
    val s = toStr(sum)
    print(s); print("\n")
