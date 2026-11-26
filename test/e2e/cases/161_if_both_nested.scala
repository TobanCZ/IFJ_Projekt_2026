import ifj26.*

def test(a: Int, b: Int): Int =
    if a > 0 then
        if b > 0 then
            return 1
        else
            return 2
    else
        if b > 0 then
            return 3
        else
            return 4

@main def main(): Unit =
    val neg = -1
    val r1 = test(1, 1)
    val r2 = test(1, neg)
    val r3 = test(neg, 1)
    val r4 = test(neg, neg)
    print(r1); print(r2); print(r3); print(r4); print("\n")
