import ifj26.*

@main def main(): Unit =
    val x = 2 + 3 * 4 - 10 / 2
    val y = (2 + 3) * (4 - 10) / -2
    if x + 1 == 10 then
        if y * 2 > x then
            print(x); print(" "); print(y); print("\n")
        else
            print("NO1\n")
    else
        print("NO2\n")
