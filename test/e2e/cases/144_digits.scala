import ifj26.*

def digitSum(n: Int): Int =
    var x = n
    if x < 0 then
        x = -x
    else
        ()
    var sum = 0
    while x > 0 do
        val q = x / 10
        val d = x - q * 10
        sum = sum + d
        x = q
    sum

def revInt(n: Int): Int =
    var x = n
    var sign = 1
    if x < 0 then
        sign = -1
        x = -x
    else
        ()
    var rev = 0
    while x > 0 do
        val q = x / 10
        val d = x - q * 10
        rev = rev * 10 + d
        x = q
    rev * sign

@main def main(): Unit =
    val n = -12345
    val s = digitSum(n)
    val r = revInt(n)
    print(s); print(" "); print(r); print("\n")
