import ifj26.*

@main def main(): Unit =
    val a = (10 + 20) * 3 - 8 / 2
    val b = 9.0 / 2.0 + 1.5
    val c = strLength("hello world!")
    val sb = toStr(b)
    if 10 > 5 then
        if 3 == 3 then
            print(a); print("|"); print(sb); print("|"); print(c); print("\n")
        else
            print("NO\n")
    else
        print("NO\n")
