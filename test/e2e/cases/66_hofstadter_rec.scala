import ifj26.*

def female(n: Int): Int =
    if n == 0 then
        return 1
    else
        val n1 = n - 1
        val fn1 = female(n1)
        val m = male(fn1)
        return n - m

def male(n: Int): Int =
    if n == 0 then
        return 0
    else
        val n1 = n - 1
        val mn1 = male(n1)
        val f = female(mn1)
        return n - f

@main def main(): Unit =
    var i = 0
    while i < 7 do
        val f = female(i)
        val m = male(i)
        print(f); print(","); print(m); print(" ")
        i = i + 1
    print("\n")
