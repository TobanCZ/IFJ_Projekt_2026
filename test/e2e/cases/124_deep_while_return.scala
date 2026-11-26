import ifj26.*

def findTriple(target: Int): Int =
    var a = 1
    while a <= 5 do
        var b = 1
        while b <= 5 do
            var c = 1
            while c <= 5 do
                if a * 100 + b * 10 + c == target then
                    return a + b + c
                else
                    c = c + 1
            b = b + 1
        a = a + 1
    return 0

@main def main(): Unit =
    val keep = 777
    val r1 = findTriple(245)
    val r2 = findTriple(999)
    print(r1); print(" "); print(r2); print(" "); print(keep); print("\n")
