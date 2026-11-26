import ifj26.*

@main def main(): Unit =
    if "a" < "aa" then
        print("1")
    else
        print("0")
    if "Z" < "a" then
        print("1")
    else
        print("0")
    if "0" < "A" then
        print("1")
    else
        print("0")
    if "" <= "" then
        print("1")
    else
        print("0")
    if "abc" != "abd" then
        print("1")
    else
        print("0")
    if "#" < "$" then
        print("1")
    else
        print("0")
    print("\n")
