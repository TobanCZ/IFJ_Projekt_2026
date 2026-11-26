import ifj26.*

/* ============================================================
   Zatezovy E2E test pro IFJ26 (scanner + parser + PSA + pamet)
   /* Vnoreny /* vicenasobny */ komentar */
   ============================================================ */

// Jednoradkove funkce s i bez navratoveho typu
def square(x: Int): Int = x * x
def negate(x: Double) = -x
def nothing(): Unit = ()

// Rekurzivni funkce (GCD)
def gcd(a: Int, b: Int): Int =
    if b == 0 then
        return a
    else
        var x = a
        var y = b
        while x >= y do
            x = x - y
        val res = gcd(b, x)
        return res

// Hluboke vnorovani bloku a slozite PSA vyrazy
def heavyMath(n: Int, factor: Double, label: String): Double =
    var acc: Double = 0.0
    var i: Int = 1_000
    val bigInt = 1_234_567
    val tiny = .25
    val sci1 = 1.5e-3
    val sci2 = 2_0.0_5E+2D
    val emptyStr = ""

    // Slozity vyraz s unarnim minusem a vnorenyma zavorkama
    val complex = (
        -((i + bigInt) * (n - 3))
        + (--n * -2)
        - (n / (1 + 1))
        )

    acc = acc + tiny + sci1 + sci2 + factor
    acc = -(-acc) - -1.5

    var outer = n
    while outer > 0 do
        var inner = outer
        while inner > 0 do
            if inner == outer then
                if inner <= 5 then
                    acc = acc + 1.0; inner = inner - 1
                else
                    acc = acc * 1.05
                    inner = inner - 2
            else
                if inner != 1 then
                    acc = acc - 0.25
                else
                    acc = acc + 0.1
                inner = inner - 1
        outer = outer - 1

    print(label)
    print(emptyStr)
    print(complex)
    return acc

// Funkce s vice parametry a viceradkovym retezcem
def formatReport(
    title: String,
    a: Int,
    b: Int,
    ratio: Double
    ): Unit =
    val header = """=== REPORT ===
"Quotes" and ""double quotes"" inside
Line 3 of triple string"""
    val escaped = "Tab:\tNewline:\nCR:\rSlash:\\Quote:\""

    print(header)
    print(escaped)
    print(title)
    print(a)
    print(b)
    print(ratio)
    return

@main
def main(): Unit =
    val a = readInt()
    val b = readInt()
    val d = readDouble()
    val s = readString()

    val g = gcd(a, b)
    val sq = square(g)
    val neg = negate(d)
    val u1: Unit = ()
    val u2 = nothing()

    // Viceradkove volani funkce (continuation lines)
    val result = heavyMath(
        sq,
        neg,
        s
        )

    formatReport(
        "Vysledek vypoctu",
        g,
        sq,
        result
        )

    // Sekvence prikazu na jednom radku
    var k = 0; k = k + 1; k = k * 2; print(k)

    print(u1)
    print(u2)
    return ()