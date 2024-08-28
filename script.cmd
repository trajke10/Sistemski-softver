flex ./misc/flex_vezba.l 
bison -d -t ./misc/parser.y
g++ -o asembler ./misc/lexer.cpp ./misc/parser.cpp ./src/Asembler.cpp ./src/mainAsembler.cpp ./src/Symbol.cpp ./src/Literal.cpp ./src/Section.cpp
