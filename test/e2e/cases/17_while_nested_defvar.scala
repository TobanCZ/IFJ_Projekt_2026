import ifj26.*

@main def main(): Unit =
    var i = 0
    var sum = 0
    while i < 5 do
        val sq = i * i
        if i == 2 then
            val bonus = 100
            sum = sum + sq + bonus
        else
            var tmp = sq + 1
            sum = sum + tmp
        i = i + 1
    print(sum); print("\n")
