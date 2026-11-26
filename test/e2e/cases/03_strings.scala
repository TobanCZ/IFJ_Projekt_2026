import ifj26.*

@main def main(): Unit =
    val s1 = "Ahoj" + " " + "svete"
    val s2 = "Tab:\tNL:\nCR:\rSlash:\\Quote:\""
    val s3 = ""
    val multi = """A
hoj s\nvete
"quoted" """
    val n = strLength(s1)
    val sub = subStr(s1, 5, 10)
    val ch = chr(65)
    val neg = -1
    val o1 = ord(s1, 0)
    val o2 = ord(s3, 0)
    val o3 = ord(s1, 99)
    val o4 = ord(s1, neg)
    print(s1); print("\n")
    print(s2); print("\n")
    print(multi); print("\n")
    print(n); print(" ")
    print(sub); print(" ")
    print(ch); print(" ")
    print(o1); print(" ")
    print(o2); print(" ")
    print(o3); print(" ")
    print(o4); print("\n")
