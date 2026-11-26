import ifj26.*

def tak(x: Int, y: Int, z: Int): Int =
    if y < x then
        val x1 = x - 1
        val y1 = y - 1
        val z1 = z - 1
        val a = tak(x1, y, z)
        val b = tak(y1, z, x)
        val c = tak(z1, x, y)
        val r = tak(a, b, c)
        return r
    else
        return z

@main def main(): Unit =
    val r = tak(12, 6, 3)
    print(r); print("\n")
