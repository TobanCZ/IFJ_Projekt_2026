import ifj26.*

def collatzSteps(start: Int): Int =
    var n = start
    var steps = 0
    while n != 1 do
        val half = n / 2
        if half * 2 == n then
            n = half
        else
            n = 3 * n + 1
        steps = steps + 1
    steps

@main def main(): Unit =
    val s1 = collatzSteps(1)
    val s2 = collatzSteps(27)
    print(s1); print(" "); print(s2); print("\n")
