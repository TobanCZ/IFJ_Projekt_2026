import ifj26.*

def isEven(n: Int): Int =
    if n == 0 then
        return 1
    else
        val m = n - 1
        val res = isOdd(m)
        return res

def isOdd(n: Int): Int =
    if n == 0 then
        return 0
    else
        val m = n - 1
        val res = isEven(m)
        return res

def addOne(x: Int) = x + 1

def multiImplicit(x: Int): Int =
    val y = x * 2
    y + 1

def unitWithExpr(): Unit = 123

def unitWithReturn(): Unit =
    return 456

@main
def customEntry(): Unit =
    val e = isEven(6)
    val o = isOdd(6)
    val a = addOne(10)
    val m = multiImplicit(4)
    val u1 = unitWithExpr()
    val u2 = unitWithReturn()
    print(e); print(" ")
    print(o); print(" ")
    print(a); print(" ")
    print(m); print(" ")
    print(u1); print(" ")
    print(u2); print("\n")
