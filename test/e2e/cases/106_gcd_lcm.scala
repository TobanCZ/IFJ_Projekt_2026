import ifj26.*

def mod(a: Int, b: Int): Int =
    val q = a / b
    a - q * b

def gcd(a: Int, b: Int): Int =
    var x = a
    var y = b
    while y != 0 do
        val t = mod(x, y)
        x = y
        y = t
    x

def lcm(a: Int, b: Int): Int =
    val g = gcd(a, b)
    (a / g) * b

@main def main(): Unit =
    val g1 = gcd(48, 18)
    val l1 = lcm(48, 18)
    val g2 = gcd(101, 103)
    val l2 = lcm(12, 15)
    print(g1); print(" "); print(l1); print(" "); print(g2); print(" "); print(l2); print("\n")
