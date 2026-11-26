import ifj26.*

@main def main(): Unit =
    if 1 < 2 then
        print("1")
    else
        print("0")
    if 2 <= 2.0 then
        print("1")
    else
        print("0")
    if "abc" < "abd" then
        print("1")
    else
        print("0")
    if "abc" >= "abc" then
        print("1")
    else
        print("0")
    if () == () then
        print("1")
    else
        print("0")
    if 3 != 3.0 then
        print("0")
    else
        print("1")
    print("\n")
