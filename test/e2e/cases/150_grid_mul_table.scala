import ifj26.*

@main def main(): Unit =
    var diag = 0
    var total = 0
    var i = 1
    while i <= 4 do
        var j = 1
        while j <= 4 do
            val cell = i * j
            total = total + cell
            if i == j then
                diag = diag + cell
            else
                ()
            j = j + 1
        i = i + 1
    print(diag); print(" "); print(total); print("\n")
