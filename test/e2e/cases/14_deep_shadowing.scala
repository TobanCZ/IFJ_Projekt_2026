import ifj26.*

@main def main(): Unit =
    var x = 1
    if x == 1 then
        val x = 2
        if x == 2 then
            var x = 3
            if x == 3 then
                val x = 4
                print(x); print(" ")
            else
                ()
            x = x + 10
            print(x); print(" ")
        else
            ()
        print(x); print(" ")
    else
        ()
    while x < 0 do
        print("NEVER")
    print(x); print("\n")
