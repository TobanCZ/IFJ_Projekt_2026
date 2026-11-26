import ifj26.*

@main def main(): Unit =
    var x = 10
    if x == 10 then
        val x = "str"
        print(x); print("|")
        var i = 0
        while i < 1 do
            val x = 2.5
            val sx = toStr(x)
            print(sx); print("|")
            if i == 0 then
                val x = ()
                print(x); print("|")
            else
                ()
            i = i + 1
        print(x); print("|")
    else
        ()
    print(x); print("\n")
