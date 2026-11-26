import ifj26.*

@main def main(): Unit =
    var a = 1; var b = 2; a = a + b;
    if a == 3 then
        a = a * 10; b = b + 1;
    else
        a = 0;
    while b > 0 do
        a = a + 1; b = b - 1;
    print(a); print("\n");
