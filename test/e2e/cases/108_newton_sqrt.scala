import ifj26.*

def sqrtApprox(x: Double): Double =
    var g = x / 2.0
    if g == 0.0 then
        return 0.0
    else
        ()
    var i = 0
    while i < 20 do
        g = (g + x / g) / 2.0
        i = i + 1
    g

@main def main(): Unit =
    val a = sqrtApprox(2.0)
    val b = sqrtApprox(100.0)
    val c = sqrtApprox(0.25)
    val sa = toStr(a)
    val sb = toStr(b)
    val sc = toStr(c)
    print(sa); print(" "); print(sb); print(" "); print(sc); print("\n")
