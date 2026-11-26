import ifj26.*

@main def main(): Unit =
    var x = 10
    if x > 5 then
        var x = 99
        print(x); print(" ")
    else
        ()
    print(x); print(" ")

    var i = 0
    while i < 3 do
        val x = i * 10
        print(x); print(" ")
        i = i + 1
    print(x); print("\n")
