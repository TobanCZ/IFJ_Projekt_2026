// c1
/* c2 /* c3 /* c4 */ c5 */ c6 */
import ifj26.*

/* before func */
def f(x: Int): Int = /* inline */ x + 1

@main def main(): Unit =
    val/*a*/x/*b*/=/*c*/10
    val y = f(x) /* multi
    line comment as EOL */
    val z = y * 2
    print(z); print("\n")
