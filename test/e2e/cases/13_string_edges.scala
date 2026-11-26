import ifj26.*

@main def main(): Unit =
    val special = "a#b c\\d\ne\tf"
    val empty = "" + ""
    val l0 = strLength(empty)
    val sub0 = subStr("a", 0, 0)
    val c1 = chr(35)
    val c2 = chr(92)
    val o1 = ord(c1, 0)
    val o2 = ord(c2, 0)
    print(special); print("|")
    print(l0); print("|")
    print(sub0); print("|")
    print(c1); print(c2); print("|")
    print(o1); print("|"); print(o2); print("|")
    if "" < "a" then
        if "A" < "a" then
            if "ab" < "abc" then
                print("LEX_OK")
            else
                print("NO")
        else
            print("NO")
    else
        print("NO")
    print("\n")
