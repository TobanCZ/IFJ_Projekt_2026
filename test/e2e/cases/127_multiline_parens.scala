import ifj26.*

def sum5(
    a: Int,
    b: Int,
    c: Int,
    d: Int,
    e: Int
    ): Int = a + b + c + d + e

@main def main(): Unit =
    val v = (
        (
            1 + 2
            ) * (
            3 + 4
            )
        - 5
        )
    val s = sum5(
        v,
        10,
        20,
        30,
        40
        )
    print(s); print("\n")
