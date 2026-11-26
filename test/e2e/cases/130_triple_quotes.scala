import ifj26.*

@main def main(): Unit =
    val s1 = """"""
    val s2 = """a\nb""c"""
    val s3 = """one
two
"""
    val l1 = strLength(s1)
    val l2 = strLength(s2)
    val l3 = strLength(s3)
    print(l1); print("|"); print(l2); print("|"); print(l3); print("|")
    print(s2); print("\n")
