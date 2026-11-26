import ifj26.*

@main def main(): Unit =
    val v1 = 1; val v2 = 2; val v3 = 3; val v4 = 4; val v5 = 5
    val v6 = 6; val v7 = 7; val v8 = 8; val v9 = 9; val v10 = 10
    val v11 = 11; val v12 = 12; val v13 = 13; val v14 = 14; val v15 = 15
    var acc = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
    acc = acc + v11 + v12 + v13 + v14 + v15
    if acc > 100 then
        val v16 = 16; val v17 = 17; val v18 = 18; val v19 = 19; val v20 = 20
        acc = acc + v16 + v17 + v18 + v19 + v20
    else
        ()
    print(acc); print("\n")
