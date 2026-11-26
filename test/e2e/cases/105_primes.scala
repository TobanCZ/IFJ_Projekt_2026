import ifj26.*

def isPrime(n: Int): Int =
    if n < 2 then
        return 0
    else
        ()
    var d = 2
    while d * d <= n do
        val q = n / d
        if q * d == n then
            return 0
        else
            d = d + 1
    1

@main def main(): Unit =
    var count = 0
    var sum = 0
    var i = 1
    while i <= 100 do
        val p = isPrime(i)
        if p == 1 then
            count = count + 1
            sum = sum + i
        else
            ()
        i = i + 1
    print(count); print(" "); print(sum); print("\n")
