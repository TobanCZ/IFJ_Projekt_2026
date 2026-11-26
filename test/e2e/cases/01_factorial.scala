import ifj26.*

def factorial(n: Int): Int =
    var result = 1
    var i: Int = n
    while i > 1 do
        result = result * i
        i = i - 1
    result

def factRec(n: Int): Int =
    if n <= 1 then
        return 1
    else
        val m = n - 1
        val prev = factRec(m)
        return n * prev

@main def main(): Unit =
    val n = readInt()
    val r1 = factorial(n)
    val r2 = factRec(n)
    print(r1)
    print(" ")
    print(r2)
    print("\n")
