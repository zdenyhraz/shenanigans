#include <string>

namespace AoC2021D25
{
std::string input =
    R"(>v.v>vv.>.>.vv.>v>...>.vvvvv.>>v.v.v....v>v...>.>vvvv.v..v.>..v.v>>>>..>v>>......vvvv.vvvv.>...v.>.v>..v.>v...vv>v>.>vv.>>>..>v>v>.....>>..
............>..v...vv>....>..v>..>..>.vv.......v>>..vv>>..>>..v..>>.v....>vv...>vvv>vv>>v>.v>...vvvvv....v>.v..>>....>..v.vv.v>...>...v...v
...v..v>...v.vv>>.v>.vv>.vv.>..v>>.v.vvv....>v.v>...>.v>...>v.>v....v.v.>.>v>>..>.......>...>>>>v.vv>v..v...v.>..>v.....>.>vvv.>...>>>v..>>
..>........>.v..v.....>vv>..vvv..>vv......>.....>..vv.v>>v.>>....v.v.v>v.>v...>.vvv>....v>>..>.v>.v>.>>vv.>..>..vv.v.v>v>>vv.>...v...>v>...
vv.v...>.>>.>v..v..>....v.>>v.v...v.....>v.>.vv>v...v.v>v>.>>vv.v.v..vvv>.vv>.v>.>..v.v.>..vv.....>v>>....>.v>v..v..>....>>>...>v.v>.v..vv.
.>....>>..v>v.>>vv>...>>..v.>>.........v..v.v.v>..>..v>.>.>v...vv.>...>v.v.v>vv.v..>>.v>v>.>.....>v..>>..>v>..>>.....vvv.>.>..v.vv..>v.>.>.
.vv..vv>...v>..>...vv...vv..>v..v>...>.>>vvvv..v..v.v>.>>..vvv>v>.v>.v.vv.v.>.v>....v>.v..v.v.>v.....>...>v...>v.v>>......>>>.vvv.vv.>v...>
>.vv>...>v...v.v..>>.>v....vvvv>>v..v.vv>v>..>.>v.>.v>.vv...v>vv>v.>vv...v>...>.>vv.vv..v.>....>..>...>>>.....>v...v.>>v>.v.>...>....>..v.>
>>vv>....v.>..v.>...>..>v.>..>.>..v.v.....vvv>>.>.vv>.>..>...v>.....v>vv>.>>>.>.>.>.v>>>...>>v>v>v>.>>>.>..v>v.>.vv>...vvv>.v..v...v.v.>>.>
.v......>vv...>v>..>..>v>>vv..>..v.v...v..>v>..v.v>.>v>.vv.v....v>v..>.......vv>.v.>>v........>.....>vv.>..v>..>>v....>v>.v...v>v..>vv...>v
vv>v..........>...>.vv>.......>>..>>.v...vv>.>vvvv>>>.v>..v.v>v>>.v..v.vvvvv..v.>>.....vv...vv........v...vv.>>v.>...v.>...>.v>...v>>v>>.v.
..v.vv..v.....>>>>.vvv>v>>....v.v.>.......>vv>..v>v.>....>.v>v.v.>>vvvv.>>..>>>v.........>.>..v.v>v>.>vv.>..>>>vv....>>>.>vv.v.vv..........
v.....v>>>v>..v>.>.vvv..>.....>v....v>..vvv.>.v.>vv.>..>....v..v>>v.>v..>...vvvv>....v>v>v>.....>......>>.v..>v.v....v>.v>...v.vv>>.>v.vv>.
v>.....v>v..v...>.>.vvv>v>..v>.vv...>>v..vvv>......v>.v>vv..v>>..>...>.vv>....v.v......>.>>v>v..>..v..v....v.>vv>....v>>.>v.>v..>.>v..>v..v
.v>.>>v..v>......v>v>.>....v>.vv.>.v>...v.v..v.vv.>v...vv>.>>>vv.>vv....v...>.v.>..v.v.>.v>>>...>>v.v.>>>.>.>vv.>.>..>v>.>>.v.v>..>>.vvv.>.
vvv>.v.vv.....v>.v..vv...v.v.>.v.v>...v.>vvv.>v...>>v.v...vv>..v.>..v>>>..>v.>v.>.v>.v>..>>v>>.v..v.>v>.>.v>>v.>v....v.>v.>vv.>>...v..v..v.
v.v>v>....v.v.>vv.>>..v>.>>......>v.>.vv...v.vv>.v.v...v.v>....v..v>v>vv..>....>..>v.>..v.....>v.>v..v>.v..>v.>vv.v.v.>...>....vv.v.>>.>v..
..>>>vvv..>>>>v.>>.>..v.v>>>.>v.vv.......vv.>>...>v..v.>>>>v>..vv>v>...v>>.>.....v..v...v.>.>vv....>v.vv...v...>v..>v..>..v>.>.>.>>>>..>.v>
vvv>v.v..v>v..vvvvv....vvvvv..>..v>....vv..>.v...v..vvvv.>.>.vv>.v>v>>v>.>..v...vv...v.>.v>......v>>.v.>......>>.>>v>v.v....v>vv.v>.v.>>.>.
vv..>.>vv.v>.vvv..v>.>>.>v..>v.v>.v.v...v...>v>>...>v...v...vvv...vv..>>v>.v>>....>vv..>.v>v..vv..........v.>.>.>v>.v..>.>>.....v>.vv.>>>vv
..>v....>>....>......v>>v.v..v.v.v...>>.....>vvv........>>v..vv>>.v.v...v>..>vvvv.>>v>>>.>.vv...>vv...v>v.v...v..v..>..>vv.v.v>v>>vv.v.....
...>.>v.v.vv..vvv...v>..v>v>>.>>.>...v...v..>>>.>.v.>v...v...v..>...>...v..v...vv>v.vv>vvv>...v..v>>.vvv.>.>>vv...>vv.vv>>.>vvv.........>.>
.>...>..vvv>>..v>.>.v..>.>v>..v>>......vv...v...vv>vv>..v.vv.>.>v.v..v>>.>..>>v.v.vvvv>>....>..>>...v>..>v>vv.v.v>>v>>.>vv..>.v..>>.>....>.
>.v...>v.v.>.>>.v..>>v.v.v>>.>v..>v.v.>v>..vv..v.>v..>....v.>....>v.>.>v>v>>..vv..v>.v.v>v.>v>>.>>v>>....>.v.>..>..>vvvv...vv>v.>.v...>>..v
.>.>..>.>>.vvv>v.v..>v.v..vv..v.v.>v.>.....>vv..>..>v.>>>v.>v>vvvv.>.vv..vvv>>>>>v..>v>v>.>>.v>v.v..>.v.>..>..v>v.>.......vv.>>..>.>....v>v
..vv...vv..v>>vv.v..>..>.v.....>v.v..v.v.>>v>>v>......>>>.....vv.>>v>>>......>>.vv>v>>>v.>..vv....>.v>>.>.>.>>.v..>.....v.v....>>>.>>v.>>>>
.>.v>.vvv>.>..>.vv>>vv>>v.>v>.>v.v.>.v.v..v..v.>.>.>>..>..>.v.>vv.vvv.v>>>.>..>>v>....>..vv.>..>>v>v..>>..>...>.>>v.>..v>.....>>>...v.v.>v>
v.v...>>.>v.v>v...>.>..>>...>..vv>.v.>vv.>v>.>>>.>vv.>>>>....v..>..>.>>.....>.......>.v.v.vv...>..v>>>.vvv>>>v.....>>.vv..v>...>v.>>....v..
..>v>>.v.>.>>>v>.>v.v>..>>>...v>>>..>...vvv.....>>>.vv>>v....v.v>>.>...v....vv..v.vv.v>.>v>.v.v>>.v....>vv.v>v>>vvv>..v.v...v>v>>....>.>..>
.>v..>.>..vv...>v.v..v...>>.>.v>v..v.v...v>v>.>v>.>.vv>>...v.>>...>v.v...v...>..>vv.vv>..v>>v.v.>.v..v...vvv>.v>>>..>>v>..>.>..v>v..v...vvv
v>>...>>..>....v>.v.>>v..>v..vv>..>.>.>>>v>v>vv.>....v..>>..v>.>v.>.vv>>>>v...>v>>....>v.....vv>v..>..>.....>..>.>v>v>..v..>>.>.>.>...vvv>>
...>v.v>.v.>>...vv...>>..v>...v.>v.v>>.v>.v.>>>v>.....vv..>.>..v>..vvvv.>v.>>>.>.>v...v.v..>.>.v.vv....>..>.v.>>.v>..v..>..>vvv.>vv>v>>....
>....v...v.vvv..v..>vvv>>.>>.>.v.vv..v.....v>v..>..>.>.......v>...v.v..>....>v.v..v.v.>..v.v......>.v.v.>..>....v.>.>.....>...>.v>.v.v...v.
v..v>.>.vv...v>.>>....vv.>v>v..>v>v.>v>v>..v.v>v>>.>.vv..>v>v.>>..>>..v..>vv.v>.v..vv....>.v..v>>v.vv..v>>...vv>.>...>>>.>>.v..>.>v.v.>...>
.>>.>v>.>v>>vv..>>>v.>..v....>>..v.v..>v..v..>.vv....v.>...>...>.>.>...>.v>.....>v>...v.>v..>>v.v........v>...>.>.>.v.v>.vv.v..>v>.v>v>vvv>
.vv.>>v>>.>...>....v>v.>.>>....>.v..>>>.v....v..vv.v>.....vvv.>.v..vvvv..vv.>v>.v..vv.>..v...>.>>.>...v.>..v>...>.v....>.v...v>v>..>v..>>..
vv..v.>...v....>v>.>.v>>v>..>vv.v>....>.v..v..vv.......>.v.v.>...v>>...>vv>>..v.>...>....>>...vvv.v.v>>.>..v.v....>....>v.v.v>v>.....v....v
..>v.>>.v..>vv.v..>>......v.......>..v>.>.v>v...vv..v.v...>.>>...v.vv>>>..>.v>.>>v>.v.>>vv>.>>.>...>..v>.>>v>>>>>.vvvv.>.vv......>vv.v>....
.>vv.>..vv>>>>v..v>v.....>vv.>>.v.>.v.>>>v.v..v>>...>vv.v>v.>v>v.v.>v>...>>...v...v>.....v...v..>..>...v.....>.v..>.v>......v.v>...>.vv.>>v
..v>>....v>..v....>>.vv>v..vv.v>>..v...v...>.>>..v>....v..v.v.v.v>v.>...vv>>..vv....v>>>..v.v.v....v.v......v>>vv.>v....v.v..>>.v..vvv..>vv
..>vv>>>>vv>.vv....>..>..>..>>...>..v...>>v......v>...>.vv>>.v.v>....>>..>.v>v>.>v.v>.v.v.v.>v>v>..vv......>v.>>vv...v>v..v>v>...vvvvv>>>v.
>.>>>..vvv>.>v.>v.>.>v>.....v>v..vvv.v>..v>.v.....>.>..>....v.>.v>...v>..>..>>v.>vv.v..>vv>.>v..v>.v>vv.v>..>>.>>..>.vv...v...v.>..vv.v...v
v>>.>>..v..vv>.vvv...>>..>v.v..>...>..v>..>..>>..v>.>>.....>.>vv>....>..v.vv.v..v>.vv.v.v>.>...v>>...v>.v.>.>..vv..>>>>.v>.v>.vv..>>.......
...>>..>...>.v>>v..>.....>.>....>vv>v>vv>>.vvv.>.....>>.>>>vvv..v..>v>.>.v>>v.>vv>>....v>>..>>v>>.vv>.v.>v.vv>>..v........>v.v..>.....v>.v.
v...>>>v.v.v>v>.>.v....>>>..vv>v.....vv..v.>.v.v>....v.v>>.>..v.v...>.v>>..>..v....>.v>...v.v>.v..>......vvv.>vv..v....v..>v.>>v>v..v..>...
.>>v....>..v>>v.vvvvv>...>..vv.>..>.>.>v.v.v>..vv>..>vvv>.vv.>v>vv.........v........v.>.....v>...>v>.vv.v...v...vv.v.>.v..>.....v.....vv>.v
.vv..v>>v..>.v.>>v>v>....>.v.>..>>vv>.v.v.>>..vv.v>>..v>>v..v.v.v.>>.>>..vv....>.>>..>.....>v.>>>.v.v.v>v>v.>.>v>>v......>v>...>v.v.v..>...
.>>...>..vv>....>...>>.>>..v>v.....>>.v.v..>vv>.v>>.v.>>..>v>>.v..>.>vv>v>v...>..>.v>..v>v>.>v>..>>.......v..vv>v>..>v....vvv...>..>..>>.v.
>...>>>v>>.>.>>...>...>.>v.>.v..>.v....>.v>vvv.>....v....>....v.>.>..>..>.>>.>vv.vv>v>....>vv.>.v>.>..v>v.....v.vv....v..>>.>vv>..>.>....v>
>.>v...vv...v..>.vv>v>.v.v.v...v....v.vv.v>.v..>..vv.....>>.>.>>.>v.....v>......vvvvv>>vv..>...>....>..>..v.......v>.v>...vv>.....>.>>..>.>
v.v....v>.v.>>v...>vv.>v.>v>....vv>.v..>>.v>v...v>>.v>.v>...vv..vv.vv>v.>>...vv>.v..vv>.v>.>.>.v>..>.v.v>v......>.vv.v...>>>...>>.v.>v.>.v.
>>>v..>..>....>v...v.vv....>....v>.>v>v....v>>..vv>v.v..>..vvv>>v>..>.....>>v..v.v>v...>.v>v.v>.v......v...vvv.>.>.>>>v.v...>....>>>v......
v>v>.v>v.>>>..>>>v......>v>vvv...>...>>.v..>>>>....v>v.>..vv.>v.>>v....v.....>>....>vv.v...v.v>..>>>.>...v>.v..v.>.>>>....>>v.>..v..v....>.
....>>vv...v.v...>.>..vvv>v.>.v......v.....>..>v>....>>v.v.v>v>.>>v...>>.>>>>>.>.v.v>v......>..v>.>.>.>.....>..>>..>...>v.v>.>>.>...>v.v..>
..>v.>vv>..vvv...>.v...v.>vv.>.v>.>v>>v>..vv>.v.>>>.>>vv>v>>>..vv>.>v>>.....>>>.v..vvv.v.>v....v>.vv...vv>v>...v...>vvvvv>...>...v>..>vv>>v
.>vv..>.>v.....vv>..>..>...v...>>.v...>.>>.>vv.v....v>>.....>v.>>v>>..v..>>>>vv>>v.v.....v.>.>>>.>v.v>v.v..>..>...vv..>>v>.v.vvv.>.>>.vvv..
>...>vvv>vv...v>v>>..........v.....v>>.......>...v>>.>>v>.....>.....>v>..>.>..v..v.>>v>.v.v>v.>v..>>>>.>vv>..v>>..v.v..>v>.vvv..vv....vvv>.
..v.....>.....>.v.vv...>>.v>>.>>vv.>v...>.v..v>.v...v.v.vv....v>.>vv>>...v.....v..vv.vv..>......>v>>....>.v.>vv>vv>>.v.>v.v..>.>>...v..v>.v
.v...v..>.>v..>>.>>..>vv>......v>v.>..>.>....>...v>v..>.vv..v>.vv......v>>.........vvvvv>vvv.v>v.>>v>..v..>>...>>v....v...>>v..vv>...>....v
.vv>>>.v.vvvvv....v>.vv.>...>>..>v.v....v.vv..>..v....>..v>.>vv>.>.>>>>...>.>.>vv.v>>v.v.>.v.>...>vv....>.v.vvvv.v.>>v>>.v..>.v>...>.>vv...
v>v>>..>..>.>>...v>vvvvv..>vv.vv..>.....>vv.>.>v>.>>>.>>>v..vv..vv....>>v.>v.....>v>v>...v.v..v>>...>>>v.>v..>..vv..>.>>>v...>...vvv....>v.
vvv>v>v..v...>v.vvv.>..>.v.>>>..v....>v...vvv.v>.vv.>>vvv>>...>..v>>vv...>.>.>.v.>.>...vv>>..v>vv..vv....>v.>vv>...>..v>.>>...v..>>>..v.v.v
.....>>..>..>>>>.>>>v.>>.v...>v>>>v.>>..>v>>>v>>.>..vv>v.>>v.v.>>>.....>.>v..>v.>v>>.>.>>.>.vv.>>v...v>v..>v>v......>..>...v..>.vv....>..vv
v>v>v>v>v..>....>v>v.>>>>...v.....v.>.>.>v.>.v...>.>.....>v..>...>..>...v..v>.....v>.>vv>>..>.vv.v...>>v.v.>vv.>vv.>v>..vv>v>.v..v.>.vv.v..
v>......>v..v...v....>v..v..vv.....vv....v.vv..vv>v>>.>v....v.v>v..v>.>.v...>..>...>.vv.v..>....>>v>.v....>.>v>...>>>>.....v.....v.>v....>v
>>..vv....vv.v.v..vv.>.>>>v.>.>>vv...>>>>.vv>>...v.v.v>.v..v>..>.vv..>vvv>.vv.vv.v...>>.....>.v>..v.v>..>..v..>v>vv>.vvv.v.v>.vv>...>v...>.
>v.vv>>.>.v.v.v>.vv>v....>v>.>.vv..>>.>.v.v..v.>>vv>>...vv.>.v.vv.>v.>...v>v>..>.v...v...>.>v>v...v>v.....v.v....v>.v>.v..v>..>..>.>>>>.>v.
.vv.>v.v..v>.....>>>.vv...>v...vvv>.v..>..>v>>..>v.vvv>.>>v.>.vv.......>.v>..v.v.>.......v...>vvv..v.>>v..>..v.>...>.>vv.>.>.>.>.>>.>>>...>
.>v.>..>....>.v.>.>>>>v>...>.....vvv>v..v>..>v..v>.>>..>>vv>>.v>vv....v>>..v...v.v...>v..>v...v.v.v>.....>.>>...v>v.>...v.>>.v.....vvv.vv>.
....v....v>v>.v>.>.>v..>vvv>.>>..>>.>.v.>..v......vv>..v>>>.....>.v.v.>v>....v>.>.v>.>v.>v.>.>.v>>.>v.>>.>...>v..>v...>..v>...v...v....>>vv
vv>..>...>>.vvv..v.v.vv.vvv..>v>v>v...>...>>>>.....>.>>>v..v..vvv...v>....vv>>..v...vv>>vv.>>..v>v.....>.v...>..v.>v.v>>v..>.v>.>v.....>.v.
.>..>.......>..vv.>v>.>.>.>vvvv...>>.>>..>>.v..>.vv>.....v>....>......>>v.v.vv..vv.>.>v.>.....>>..>.v>.v.v..v.>..v...>....>.>>.>>>v.>>.vv..
.v.....>>v>v>...>v.>...>..v..v>.v......v>>...v.>vv.>..v....vvv..vv.v.>..>>>v>>..>.v>....>v.>>.>.>v..v>.v.>v..>....v.>..v..vvvvv.v...>vv.>v.
v.>>>>>.v>.>vv>v.v.v.>>....v.>v..v>v>..v...v.....>.v>..v..v>.>v>.v.>v.v..vvvvvv.v..v>.vv>..v.v>v>...>..>v.v...v>v...v.>v.>.v..vv.v>v.vv>.v.
..v>..v...vv.>......v>>v>.>..v..>....>>...>>..v...v.v..vv>.v..v..>.vvv..v.......v..v>v.vv>.v>.>v>.>v..>...>..>>.v....vvv>v.>...>.>v>>v...v.
>.vv..>v>.>..>v.v.......v..>.>v>>vv>>>>v....>...v>>..>v..vvv>..>>..>vv.>v..vv.v>.....v..>...v.>.>vv...>v...>v>>v...>.>v..vv....v.v.>v..v>.v
....v>.v..v..>....>>v..vv>vv>.v.v..v>.>..>.vv.>>>>vv....>.v>..v.>.vvvv.v>v.v>.......v.v....>..vv..>...>>>v.>>..v..>vv.vv>v....>.>.....v>v..
..vv...vv...v>.vvv.v..>..>..v...>....v.v.>...>v.v.vv.>...>..>..vvv...>>.>v..>..v.>.>......v.v>....>..>.>..>>.>...>>>>......v>.>..>v...>vvv.
v>v......>.>...v.>...vv.>vv.v.>..>v>v.>v.>.v>v..vvv...v>>...v.v>v>>>.v.>......>v...>..v..>..v.v.>v...vv.>.v>...>vv....>.....>v.>>..>>.>>.>v
>v.v.>..v.>v.vvv...>v>......v.>v>v>vvv..vv>.v>>v>>.....vvvv>..>v>.v.v...v.>....>.v>.v.>.....>v.v..v....v.>.>v..v.....>>...v....>>..>>>..vv.
..v..>.v>>.v.vv>.>.v.>>..>v>>v>>>.>.>.vv...v..v..v....v..>....vvv.v..v>.v>vv.vv>.>>>>.>..vv.v.....v...v..>.vvv.>.vv..>....v.>v>vv.>.vv..>.>
.v....v>v..>..v>v>..>>....v.>..v>.v.>....v.....v>>>>v..v>.>.v.v.v>.v..>..v....>.>..>vvv..>.v.>..>>vv....>.v.vv.v.v..v...>vv.v...>....vvv>>.
.>v>.v.v.v>>.>..v.....v>>>>.v>>v.>.....v.>>>>.v..vv.>..>.....vv>>v.>vvv.>v..>>....v.>v.vv..vv..v.....>.v>.vvv..v.>v>....v.>>......>>.>v..v>
>...v.>v.>>...>v..v...v....v>vv>>>..>>v....>.>..v..vv>.v.vvv..>....>...v.v>.>.>v.>v>>...vv..v..vv>..>vvvv..v>...vv...>..vv....v..v..>.>...v
.>>..>...v>..v.>.v>..>..>...v...v>v>.>v...v..v...>>.v.v>vv>v.v..>...v>......v....v..>.vvv.>>.v.v.vvv>>>..vv..v..vv.v>.vv...>.v...>v>v..v.v.
v...v.....vv...v.v.>>.v.vv...v.>>......v.>v...v...>..v..>>..>....v...>v>...v>v.vvv.v...>v>v>..v....v........>v...v>...>vv.>>>>.v.>vv.v....v
..>.v..v>....>...>..v>........>.>.>v>...>v>vv>>.>...>>v>..v.>.v.>.v>>>.v.vvv........>>.>>vv..>v>......v.>vv.v.v>v.>>.>..v.>>>>v>>..vvv>.>v>
.>>v..v..vv..vv..v.>v>v.>.vv>.>.>v...>..vvv>>v.....>.>.vv>v>..vv.vv>v>>....>.>>.....vv>v>.v.v.>...>...>.>>.>.>.v.v..v.>v.>........v.>>.v...
.v......v>v>>.v.v>v...v...>>v..vv..>..v.......v>v..>...v>..>v.........>>.....>v.>>..>...>>..v..vv.>...>.>v....>v..>>>..>..>vvv.......>..>>.
v.....v.....>..>v.>.vv.>>.>v..v>>vv.>v>>..>....v>>..>v.>>v..>.v...>....v..v.>.>v.vv....>..v......>>.v..v....vvvv.>..>.vv.>v>..>>....>v>v.>>
..>..v.>.vv>vv.>..v.v..>v>v>>>....vv.....>.>.vvv.v>>.v......>>v>v>vv.>..v.vv.>vv>..vv..v>>.>.v>.vv>v.>>...v>vv.vvv>>...v>.>>.vvvvvv>.v>v...
>v>>v>.vv>>.v>v..v>..vv...>.vv.vvv>..vv.v>.v..v.>..vvv...>.v.v.v.....vv...>.>..>>.>.vv.....>v.>.v.>..>....>.v>.vv.>v..>...>>>...v.>.>.>>v..
>.>.v...>>.v..>.v.>.v..>..>v.>.v.v..vvv.v>.>.>..v.>...v..>..>v.>..v>>.v>...v...v>.>>..v>v..>.....v>v.v.>.>>.vv>v...v.vv.v..v..>.>.>>v..>..>
...>.v.vv.>v......>>v.v>..>.>>..>..>v..v.>v...v..>....>..v...>>.v>.v.>v.v.v...v....>.v>.......v.>.>>v>.vv>.v.v>>.v.v....vv>.>.vv....vv.v.>>
>.v.v.>v.>.>>vvv.>...>vv.>...>..>>...vvv>.>vv.v...v.>v..vv....v...>.v>.>.vv>..>.vv>.v>..vv..>....>.v.>.v>.v.>>.>>.v.>v.vv.v.>>.vvvv>v..v.>v
.>.>..vv>....vv>..v.v....>>>>.vv.vvv.v...>>>>v>.v>vv>...>.>.>>.v..v.v...>...v...v....v..>vv.>...vv.>......>.>..v..>>v>>....v...>>>>.>..>.>.
>v..vv....>..>>>v.>v.v.v.>.......>>>v.>v.v.>v..v.>>.v.v.v.v>.>vv..>..>.>.>>>..v...>vvv>>v.>>..v..>.>>.....>v.v..v>>v.v.>.v.v.>v.>.v.>...>v>
>>..>>v.>.v>>v>>.vvv...>.>..v>.v.>>.vv.........vv.>.vv.....>>v..>>>...>...>..v..v.>>v.v..v>..>>v.>..>>...vv>.vv...v>>v>vvv>v....>..vv.>.v.>
v>......>>vv>.>v.vvv..>>v.vv>>>>...>.>..v..v..>..v.>>...>>.>v>.>..v>...>v..v.vv.v..>>.>.>..>..>>.>>vv..>>..>..>.>......>vv.>>.v.....vvvv..>
>>...vv.>v.v>>.>...>.v>......>>>.>v>>.>..v.v...v..>v>v.v.v.v>...>...v>.v>v.>.vvv...v>.....v>.v>..vvvvv.....>v.v..v.>>.......>v>....>.>.>.v>
>>.>.v>>>..v.>.>.>.>>>.vv>vv>v.>..v...>>v...>.>.>>.>.....>.v...vv..v.vv.v>..v>>>vv.>>vv..>.>.>v.v....v...>..>>v..v.>..v..>..>.v..v>v...>...
.>>..v....>vvv.vv>v>>.v...>vv..v.>>v.>>..v.>.vv.>...>.>.v..>..>.>.>..vvv.v..>vv.>>>>...>v>>.>.>vv>.>..>>v.>..v.>>..>v>>...v..v...>>..>..>.>
.v.>v....>.......>.>>>>>...vv...>.>>v.>.>v....>....v.vv.>vv.v...v>..>v.v>.v>v>>>.v.v.>.>.v>>.v>.v.>v>>>.v>vv.>.>v.vv>...v..v..v>....>v>v>..
..v.>vv........v>v.vvvv.v..vv.v.vv..>.v..>v.v>>>...>..>.v..>v>>.....>>>.v.>vv>v.>.>v.v>..vvv...v.v>.v.>>.>.v>.v..v>v....>v>>v.>.v.v..v.>.v.
v.vv.>>....>v.v..v..>..>v...>>.......>v>..v>...v.>>..>.v..>..>v..vvv>v>..>v...v>>.>vv..v>>.v......>.v>....vv...>.>.vv..v..v.v.>...>v....>v>
.v.>>..>>v.v.>.v.>..>>v..v....v>vvv.>v...>.>.v.v.>.v..vv.vv.v.>v.v>>.v>.......>>vv...v>v>>.>v..>.v>..vv.>.>vv.v>v......>.v>.>.v.>>v...>>vv>
...>.>>.vv>v>>v.vvv>v.>..v>>..v>vv..v>..>vv.v...>>v.>vvv>>...v....vv.v....vvv.v.v.>..v.>v.>.vv.>v>.v.vvv..v..vv.>v.vv.>.>vvvv.v.>.v.....>..
v.....>>.>.>..v>...>..>>vv.v.vv..>vv.v>v>.>v>>vv...>.v>>.....>>v>>vv...>>v.>.v>.v>>vv.>>>v>......>.....>>v>>vvv>v.>v>....v....vv>...>.vv.vv
.v...>vv..>..vv.>v...>.>..v.>v.....>v>v>.>.v>.....v>v.>v.>>v.>v>>v...>.>.v..>..>.....vv>.>vvv>>..>.>.v...v.v...v.v>>>v>.>v...vvv>.v......>.
...v...v.>>...>..v...>..v>.....>v>vvv..v.v.vv>.vv..v...>....>..>v..v.>vv....v..v>vvv.....v>>.vv.>....vv>v.v.v....>..>>..>v>>.>vvv.>....>...
.v>v.>.v...v...>v.>.....>v..v>.>...........v.v......v...v.vv>v>vv>..>.v>>v>>..v..vv.vv.>.vv>>..vv>...v..v>>>>v>>>v>.>>..v.>.v>..>.>.vv>v.>v
>>>.vv.>.>vvv..v.>.vv...>.vv.v..v.v...vv.v>.>.v.v.>>>.>...v..>..>...>.vv.>vvv..>..v.v>vv....>.>v.>v....>>v>>v..>v.>>.v...>.vv>vv...>>v>v>v.
>..v.v>.v>..>.v.>>.......>v>>..v...vv....v>v>>....vvvv>.vv.>.v.....>>.>>.>.v.>>.v...v..>v>>.v....vv..>>.>...>>.vv.v.>vvv.vv>..vv....>>>.v>.
.v.>.v>.>>>v.>.vvv..>....v>..v.>...v>.....>..>>>v>>.>>>...v>>...v>v>.>vv.>.v>.v.v>.>..v.>.v...v..>>.v>..>v>v....v>.>.....>..v...v.>..v.>.>.
.>.v>.v>>....>.v...v>.>...>v.>>v..>v.v>v>vvv>>>.>>>vv.>..v...>>>..v>.v...v...>v>........v....>..>>.>v.v.>.....v>..>vv.v.>....v>....>.>.>>..
.>..v.vv>.v...v...>>v...>>..v>>..v>..v>>.>..>>.vvvv>v>vvv>v>>.>.>....vvvvv>v..>>.......>v>...>v.vv.vv..>..v....v..v.>v>>v.>.v.vv.>.>.>>v>.v
...v...>..>.v>...>>vv>.>v.vv..v...>.>v>v>.>v..v.>....v>.>>.>>vv.>>.>.>vv.v.v..>.v>v........>v>v>..>......>vv>>>.vv....v>>>>..>>>.>v..>...>v
>v.v.>v.v..v.>.v>.>...>>vvv.v>.v>.>>>.v>....>>>>.>>v...v>.>..v.>.>v.v.>>>.....v.>...vv>..vvvv>.>.v>.v>.>vvvv>>..>>vv.>..v..>v..v...>.vv....
vv>v.v.>.>..>.v.v...>>.v>>.v..>>v>>>v>>.v....>....>.>v.....vv>.....>>>..>.v>>>.v.v.vv.v>>.>.>.v.>>..v.>>..v>..>..v.>.>vv>..>v.>v>vv..>.vv>v
>.v.vv.>>>.v>....v>>v...v.....>..v.vvv..>.vv.>>.>.>.vvv>v....>>..vv..>...v.>v.>v.v>>>>...v..v..v>....>.>.....>.v....v.v.vv.v>...v..>v>.vvvv
...v>>.....vv.......v.v......v.vv.v....v.>>.>v...>.>>vv>..>>.....>>v.>.v>..v.>vv>>..>.v....vv.........>...>.v.>.v...v>vv...vv.....>.v>..>..
.v......v>>>.vv>>v>v.>v.>.v..>.>....>>>..>>...>..v.>.vv.>...>>>...vv...>>.>..v>.v..>>.>.>.v....>.vv>v.>v..v.>...vv.>>...v..v.v.>....v...v..
>vv....v>.>..>v....v.>.>v>v.>....vvvv..v>>>..>vv.>..v.>.>.>..>>....v>>>vv>.v..v>v..v..>.vv..v.v.vv.vvvvv>v..v.v>..>..v>v.>..v...v.vvv..vv..
.....>.>v>.vv..v..>v.v.v...v>.v.v....>>v.vv>>v..>>.....vvv..v.....>.v.>.v...>..v.v...>>>>..>v>>>>....>v>>.......vv>v.>>>...>..>>.v.v..>>.>.
..v.>v..vv.>....>.v>vv>>.vv.>>>..v.>v..>..>>v..vv>..v...>>...v.vv..v.....v.....vv>.v..v.>>..v..v....v>vvv..>>v..>v>>.>.v>..vv.v.>>..>v..>>>
..v.v......>v...>>>v>vv...v...v.>...>>..v.....>..>..v.v>v.>..........v.v.>v>..v..v.v.......v.>.v..v>>v..>..v.v..v>v.vvvv..>..vvv..>.....v.v
....>>.>>...>.>v>>.>>.>vv.vv..v...>..v.....>>v>>..........>..>>>>.v...>....v..>vv>>>>.>v.>>....>>>>>...>.v.....vv....>.>>>v.....v..v>vv>v.v
v...v>.v.v>>v.......v>.......>......>.v>>v>v...vv.v>....v.v.v..>..v>.v....v>.>.v....>>>..v.v..>v>.>v.vv>vv.vv>.>v..vv>v..>v.vv....>.>.v>...
>v.>.>>.v>v>........>>vv>vv.v.>..>.vv..v..>v..v..vv>.v>.v.v.>>..v>vv>>>>v.....>v>..v>..>...v..>....v>>.v.v...>v..vv.v.>v..>>v.......v.>vv.>
.v.>.>v.vv.v>.>>>...v>v.>.v...>...>.>>>.vvv>v..>.>..v>>>v...>...vv.v.>v...v.v.......>..vvv....>..>..>..v>...vvvvv..vvv.>>.>>>>...vv.>.>>.v>
..v.>vv>.>>v>>.v>v>.v>>>...>>.>v..>v.>v.>...v.>.vv.v.>v>>.>....vv>>>v.v..v.v>v>v.>>>v...>>>.>v.v..v>>>v>>...vv.vv>>v>v.v>.vv>.v.>.v>..v.v.>
>..vvvvv.>.v>.vv>...v>v.....>v..v>.>.>..v.v.>v..vv...vv..v....>v>>>v>..>..v>v>.>v.>..>>>..>>>v>>.v..vv.v.>..vv>.>>>..vv.>>..v.....>vv>v..v.
v.v>.>..>>.vvv....>...>v...vv>>.vv.>>.vv.vv..v.>.....>..>>>...v..>....>..>>..v.v.....v>.>>>.v>.>.>>>.v.....vv>>.>>v..v.>.>>...>.v.>v>...vv.
>>>v>>.......vv.v.>...v.>>.v>.>.>v>>.>....v.v>..>.v.>.v>v.v...>....>>vv>vv>>vvvv>v..>vv>v.v....>v>..v>>>.vvv.v....v.>.>>.>vv>.vv>v.v>>.>>vv
..>v>vvv.v.v>.>v..>v.vv..>.vvv.>.v>>..>..>>>>>.>.>.v..>..vv.v.v..vv>>vv..>v...>..>vv...>...>>>v....>>.v.>>v>>.vvv..>v.v.>..>..>......v>..>.
vv.v>.>.>v>vvv.>..v>vv....v.>.v..v>..v.....>.v..v.v...>...>.v.>.v..>>..v.v.vv...>v.>v....vv.v.>.vvvv..v.v>....>.vv>..vv.>.>>..vv.>.vvv.>.>.
..v.v.>>v>..>>..>>..v.>.>...v...v...v.>....v>>.>...vvvv>v...v.>>.v>..v.v..>v.v>>.v>...v.>v>>...v.v>>v>v.vv>>.>>..>v>>>>>..v.>.....>.vv.>>..)";

std::string inputDebug = R"(..........
.>v....v..
.......>..
..........)";

struct Seafloor
{
  Seafloor(const std::string& str) { Parse(str); };

  enum Entity : u8
  {
    None,
    Right,
    Down,
  };

  std::vector<std::vector<Entity>> currentmap;
  std::vector<std::vector<Entity>> nextmap;
  cv::Mat pic;
  i32 width = 0;
  i32 height = 0;
  i32 steps = 0;

  void Parse(const std::string& str)
  {
    i32 x = 0;
    i32 y = 0;
    std::vector<std::tuple<cv::Point, Entity>> cucumbers;
    for (const auto character : str)
    {
      if (character == '\n')
      {
        y++;
        width = x;
        x = 0;
        continue;
      }

      if (character == '>')
        cucumbers.emplace_back(cv::Point{x, y}, Right);

      if (character == 'v')
        cucumbers.emplace_back(cv::Point{x, y}, Down);

      x++;
    }
    height = y + 1; // string does not end with newline

    currentmap.resize(height);
    for (auto& row : currentmap)
      row.resize(width);

    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        currentmap[r][c] = None;
    nextmap = currentmap;

    for (const auto& [position, entity] : cucumbers)
      currentmap[position.y][position.x] = entity;

    Debug();
  }

  cv::Point GetShift(Entity entity)
  {
    switch (entity)
    {
    case Right:
      return {1, 0};
    case Down:
      return {0, 1};
    default:
      return {0, 0};
    }
  }

  std::string GetString(Entity entity)
  {
    switch (entity)
    {
    case Right:
      return ">";
    case Down:
      return "v";
    default:
      return ".";
    }
  }

  bool EntityStep(Entity ent)
  {
    for (i32 r = 0; r < height; r++)
    {
      for (i32 c = 0; c < width; c++)
      {
        if (currentmap[r][c] == None)
          continue;

        if (currentmap[r][c] != ent)
        {
          nextmap[r][c] = currentmap[r][c]; // not your turn - stay
          continue;
        }

        const auto shift = GetShift(ent);
        const i32 nr = (r + shift.y) % height;
        const i32 nc = (c + shift.x) % width;

        if (currentmap[nr][nc] == None)
          nextmap[nr][nc] = ent; // move
        else
          nextmap[r][c] = ent; // blocked
      }
    }
    return SwapMaps();
  }

  bool SwapMaps()
  {
    bool moved = currentmap != nextmap;
    currentmap = nextmap;
    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        nextmap[r][c] = None;
    return moved;
  }

  bool Step()
  {
    bool movedRight = EntityStep(Right);
    bool movedDown = EntityStep(Down);
    steps++;
    Debug();
    return (movedRight or movedDown) and steps < 1e5;
  }

  i32 GetStabilitySteps()
  {
    while (Step())
    {
    }
    return steps;
  }

  void Debug()
  {
    pic = cv::Mat::zeros(height, width, CV_32F);
    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        pic.at<f32>(r, c) = currentmap[r][c];

    if (width < 50 or height < 50)
      cv::resize(pic, pic, cv::Size(width * 10, height * 10), cv::INTER_NEAREST);

    Plot2D::Set("cucumbers");
    Plot2D::Plot(pic);
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (width > 10 or height > 10)
      return; // too long for text output

    LOG_DEBUG("Step {}:", steps);
    for (i32 r = 0; r < height; r++)
    {
      std::string line;
      for (i32 c = 0; c < width; c++)
        line += GetString(currentmap[r][c]);
      LOG_DEBUG("{}", line);
    }
  }
};

u64 AoC2021D25()
{
  return Seafloor(input).GetStabilitySteps();
}
}