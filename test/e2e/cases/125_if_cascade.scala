import ifj26.*

def classify(n: Int): String =
    if n < 0 then
        return "neg"
    else
        if n == 0 then
            return "zero"
        else
            if n < 10 then
                return "small"
            else
                return "big"

@main def main(): Unit =
    val n5 = -5
    val s1 = classify(n5)
    val s2 = classify(0)
    val s3 = classify(7)
    val s4 = classify(42)
    print(s1); print(" "); print(s2); print(" "); print(s3); print(" "); print(s4); print("\n")
