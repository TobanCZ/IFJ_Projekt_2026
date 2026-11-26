import ifj26.*

/* jednoradkovy /* vnoreny */ komentar */
def sum3(
    a: Int,
    b: Int,
    c: Int
    ): Int =
    val r = (
        a
        + b
        + c
        )
    r

@main def main(): Unit =
    val x = sum3(
        1,
        2,
        3
        )
    /* vice
    radkovy */
    val y = x + 4; val z = y * 2; print(z); print("\n")
