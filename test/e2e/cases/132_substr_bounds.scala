import ifj26.*

@main def main(): Unit =
    val s = "abcdef"
    val len = strLength(s)
    val lm1 = len - 1
    val a = subStr(s, 0, 0)
    val b = subStr(s, 0, len)
    val c = subStr(s, lm1, len)
    val d = subStr(s, lm1, lm1)
    val e = subStr(s, 2, 2)
    print("["); print(a); print("]["); print(b); print("][")
    print(c); print("]["); print(d); print("]["); print(e); print("]\n")
