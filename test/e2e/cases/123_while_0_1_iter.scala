import ifj26.*

@main def main(): Unit =
    var i = 0
    var sum = 0
    while i < 3 do
        var j = 0
        while j < i do
            sum = sum + 1
            j = j + 1
        i = i + 1
    var k = 0
    while k < 1 do
        sum = sum + 10
        k = k + 1
    print(sum); print("\n")
